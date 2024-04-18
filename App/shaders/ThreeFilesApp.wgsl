struct Fragment
{
	@builtin(position) position: vec4f,
	@location(0) worldCoord: vec4f,
	@location(1) textureCoord: vec3f
}

struct CameraData
{
	model: mat4x4<f32>,
	view: mat4x4<f32>,
	projection: mat4x4<f32>,
	viewInverse: mat4x4<f32>,
	projectionInverse: mat4x4<f32>
}

struct LightData
{
	position: vec3<f32>,
	_alignment01: f32,
	ambient: vec3<f32>,
	_alignment02: f32,
	diffuse: vec3<f32>,
	_alignment03: f32,
	specular: vec3<f32>,
	_alignment04: f32
}

struct Ray
{
	start: vec3<f32>,
	end: vec3<f32>,
	direction: vec3<f32>,
	length: f32
}


// Default bindings
@group(0) @binding(0) var<uniform> camera: CameraData;
@group(0) @binding(1) var<uniform> cameraPosition: vec3f;
@group(0) @binding(2) var samplerLin: sampler;
@group(0) @binding(3) var samplerNN: sampler;
@group(0) @binding(4) var<uniform> fragmentMode: i32;
@group(0) @binding(5) var<uniform> stepsCount: i32;
@group(0) @binding(6) var texRayStart: texture_2d<f32>;
@group(0) @binding(7) var texRayEnd: texture_2d<f32>;

@group(1) @binding(0) var textureCT: texture_3d<f32>;
@group(1) @binding(1) var textureRT: texture_3d<f32>;
@group(1) @binding(2) var textureMask: texture_3d<f32>;
@group(1) @binding(3) var tfOpacityCT: texture_1d<f32>;
@group(1) @binding(4) var tfColorCT: texture_1d<f32>;
@group(1) @binding(5) var tfOpacityRT: texture_1d<f32>;
@group(1) @binding(6) var tfColorRT: texture_1d<f32>;
@group(1) @binding(7) var<uniform> light: LightData;


@vertex
fn vs_main(@builtin(vertex_index) vID: u32, @location(0) vertexCoord: vec3f, @location(1) textureCoord: vec3f) -> Fragment {
	
	let out_position: vec4f = camera.projection * camera.view * camera.model * vec4f(vertexCoord, 1.0);
	
	var vs_out: Fragment;
	
	vs_out.position = out_position;
	// passing world coordinates
	vs_out.worldCoord = camera.model * vec4f(vertexCoord, 1.0);
	vs_out.textureCoord = textureCoord;

	return vs_out;
}


/*
* Checks whether the position is within out bbox basically,
* but our bbox coordinates are basically 3D texture coordinates
*/
fn IsInSampleCoords(position: vec3<f32>) -> bool
{
	var b_min: vec3<f32> = vec3<f32>(0.0, 0.0, 0.0);
	var b_max: vec3<f32> = vec3<f32>(1.0, 1.0, 1.0);

	return	position.x >= b_min.x && position.x <= b_max.x &&
			position.y >= b_min.y && position.y <= b_max.y &&
			position.z >= b_min.z && position.z <= b_max.z;
}

/*
* Sets up the ray for the fragment shader
* @param screenSpaceCoord: screen space coordinates of the fragment, used to sample pre-rendered ray start and end
* @return Ray: ray with start, end, direction and length
*/
fn SetupRay(screenSpaceCoord: vec2<i32>) -> Ray
{
	var ray: Ray;
	// Ray setup
	ray.start = textureLoad(texRayStart, screenSpaceCoord, 0).xyz;
	ray.end = textureLoad(texRayEnd, screenSpaceCoord, 0).xyz;
	ray.direction = normalize(ray.end.xyz - ray.start.xyz);
	ray.length = length(ray.end.xyz - ray.start.xyz);
	return ray;
}

fn FrontToBackBlend(src: vec4<f32>, dst: vec4<f32>) -> vec4<f32>
{
	var src_ = src * src.a; // we do not have pre-multiplied alphas
	src_.a = src.a; // don't want .a * .a

	return  (1.0 - dst.a) * src_ + dst;
}

// PRNG
fn jitter(co: vec2<f32>) -> f32
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}


/*
* Calculates Blinn-Phong shading model
* @param N: unit normal vector
* @param L: unit vector pointing from sampled point to the light
* @param V: unit view vector from point to camera
*/
fn BlinnPhong(N: vec3f, worldPosition: vec3f) -> f32
{
	var L: vec3f = normalize(light.position - worldPosition);
	var V: vec3f = normalize(cameraPosition - worldPosition);
	var R: vec3f = 2 * (N * L)* N - L;
	var H: vec3f = normalize(V + L); 

	return length(light.diffuse * max(dot(N, L), 0.0)) * 1.2 + length(light.specular * pow(max(dot(N, H), 0.0), 3)) * 1.0 + 0.3;
}


/*
* Calculates the opacity value for an illustrative context preserving effect.
* @param gradient: gradient vector of the sample, normalised in [0, 1]
* @param positionWorld: world position of the sample
* @param positionTexture: texture position of the sample
* @param startPosTexture: texture position of the start of the ray
* @param alpha_1: (alpha_{i-1}) opacity value of the sample
* Returns opacity value for the sample
*/
fn IllustrativeContextPreservingOpacity(opacity: f32, gradient: vec3f, positionWorld: vec3f, positionTexture: vec3f, startPosTexture: vec3f, alpha_1: f32) -> f32
{
		var s: f32 = BlinnPhong(gradient, positionWorld);
		// Coefficients
		var kt: f32 = 3.0;
		var ks: f32 = 0.6;

		return opacity * pow(length(gradient), pow(kt * s * (1 - length(positionTexture - startPosTexture)) * (1 - alpha_1), ks));
}

/*
* Calculates opacity with help of gradient
* @param opacity: opacity value retrieved from TF
* @param gradient: gradient vector of the sample, normalised in [0, 1]
* Returns opacity value for the sample
*/
fn GradinetMagnitudeOpacityModulation(opacity: f32, gradient: vec3f) -> f32
{
	return opacity * length(gradient);
}

// Either upload max value for dataset as uniform or upload already normalised data (had some gradient troubles)
const DENSITY_FACTOR_CT = 1/4095.0;
const DENSITY_FACTOR_RT = 1/32767.0;

@fragment
fn fs_main(in: Fragment) -> @location(0) vec4<f32>
{

	// If we would like to sample the texture with a sampler, this transforms the coordinates in ndc to texture
	// and as we rendered the cube to the texture of size of screen this gives us the coords, Y IS FLIPPED
	var texC: vec2f = in.worldCoord.xy / in.worldCoord.w;
	texC.x =  0.5*texC.x + 0.5;
	texC.y = -0.5*texC.y + 0.5;

	var wordlCoords: vec3f = in.worldCoord.xyz;

	// Ray setup
	let ray: Ray = SetupRay(vec2<i32>(i32(in.position.x), i32(in.position.y)));

	switch fragmentMode {
	  case 1: {
		return vec4<f32>(abs(ray.direction), 1.0);
	  }
	  case 2: {
		return vec4<f32>(ray.start.xyz, 1.0);
	  }
	  case 3: {
		return vec4<f32>(ray.end.xyz, 1.0);
	  }
	  case 4: {
		return vec4<f32>(texC, 0.0, 1.0);
	  }
	  default: {
	  }
	}

	// Iteration params -- Default
	var stepSize: f32 = 0.01;

 	// Position on the cubes surface in uvw format <[0,0,0], [1,1,1]>
	// apply jitter using screen space coordinates, we could divide it (jitter input) by resolution to keep it same across all res.
	var currentPoistion: vec3<f32> = ray.start.xyz + ray.direction * stepSize * jitter(in.position.xy); 
	var step: vec3<f32> = ray.direction * stepSize;
 
	var dst: vec4<f32> = vec4<f32>(0.0); // Output

	for (var i: i32 = 0; i < stepsCount; i++)
	{
		// Volume sampling
		var ctVolume: vec4f = textureSample(textureCT, samplerLin, currentPoistion);
		var rtVolume: vec4f = textureSample(textureRT, samplerLin, currentPoistion);
		var maskVol: vec4f = textureSample(textureMask, samplerLin, currentPoistion);

		// When working with gradients, we need to be careful whether we initiated pre-calculation in OnStart
		var gradient: vec3f = ctVolume.rgb;
		var densityCT: f32 = ctVolume.a * DENSITY_FACTOR_CT;
		var densityRT: f32 = rtVolume.a * DENSITY_FACTOR_RT;

		// Transfer function sampling
		var opacityCT: f32 = textureSample(tfOpacityCT, samplerLin, densityCT).r;
		var colorCT: vec3f = textureSample(tfColorCT, samplerLin, densityCT).rgb;

		var opacityRT: f32 = textureSample(tfOpacityRT, samplerLin, densityRT).r;
		var colorRT: vec3f = textureSample(tfColorRT, samplerLin, densityRT).rgb;
		
		if IsInSampleCoords(currentPoistion) && dst.a < 1.0
		{            
            // Colors
            var color: vec3f = colorCT * (1.0 - opacityRT) + colorRT * opacityRT;
            var opacity: f32 = opacityCT;
            var maskV: f32 = maskVol.r;

            if maskV > 0
            {
                color = vec3f(1, 0.5, 0.5);
                opacity = 1;
            }

            // Opacities
            // var opacity: f32 = GradinetMagnitudeOpacityModulation(opacityCT, gradient);
            // var opacity: f32 = IllustrativeContextPreservingOpacity(opacityCT, gradient, wordlCoords, currentPoistion, ray.start, dst.a);

            // Blending
		    var src: vec4<f32> = vec4f(color.r, color.g, color.b, opacity);
		
        	dst = FrontToBackBlend(src, dst); 
		}

		// Advance ray
		currentPoistion = currentPoistion + step;
		wordlCoords = wordlCoords + step;
	}

	return dst;
}


//  ------------------------------- UNUSED -------------------------------

fn ComputeGradient(position: vec3<f32>, step: f32, texture: texture_3d<f32>) -> vec3f
{
	var result = vec3f(0.0, 0.0, 0.0);
	var dirs = array<vec3f, 3>(vec3f(1.0, 0.0, 0.0), vec3f(0.0, 1.0, 0.0), vec3f(0.0, 0.0, 1.0));
	result.x = textureSample(texture, samplerLin, position + dirs[0] * step).a - textureSample(texture, samplerLin, position - dirs[0] * step).a;
	result.y = textureSample(texture, samplerLin, position + dirs[1] * step).a - textureSample(texture, samplerLin, position - dirs[1] * step).a;
	result.z = textureSample(texture, samplerLin, position + dirs[2] * step).a - textureSample(texture, samplerLin, position - dirs[2] * step).a;
	let l = length(result);
	if l == 0.0
	{
		return vec3f(0.0);
	}

	return -result/l;
}


/*
* Helper for change of base
* For now UNUSED
*/
fn ViewToWorld(pixel: vec2<f32>) -> vec3<f32>
{
	// required to pass width and height as uniforms or whatever
	var coordNdc: vec2f =  vec2f((pixel.x / 1280.0), (pixel.y / 720.0)) * 2.0 - 1.0;
	var temp: vec4<f32> = camera.projectionInverse * vec4f(coordNdc.x, coordNdc.y, 1, 1);
	var coordWorld: vec4f = camera.viewInverse * (temp / temp.w);
	return coordWorld.xyz;
}


/*
* Fixed sample size
* Based on stepSize, it returns number of samples/steps on a ray
* PROBLEM WITH NON UNIFORM FLOW...
*/
fn GetNumberOfSamples(ray_length: f32, stepSize: f32) -> i32
{
	return i32(ray_length / stepSize);
}

/*
* Based on number of samples, it returns the size of the step to fit desired number of samples/steps
*/
fn GetStepSize(ray_length: f32, samples: i32) -> f32
{
	return ray_length / f32(samples);
}