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

struct Ray
{
	start: vec3<f32>,
	end: vec3<f32>,
	direction: vec3<f32>,
	length: f32
}

@group(0) @binding(0) var<uniform> camera: CameraData;
@group(0) @binding(1) var<uniform> cameraPosition: vec3f;
@group(0) @binding(2) var samplerLin: sampler;
@group(0) @binding(3) var samplerNN: sampler;
@group(0) @binding(4) var<uniform> fragmentMode: i32;
@group(0) @binding(5) var<uniform> stepsCount: i32;
@group(0) @binding(6) var texRayStart: texture_2d<f32>;
@group(0) @binding(7) var texRayEnd: texture_2d<f32>;

@group(1) @binding(0) var textMain: texture_3d<f32>;

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
	var step_size: f32 = 0.01;

 	// Position on the cubes surface in uvw format <[0,0,0], [1,1,1]>
	// apply jitter using screen space coordinates, we could divide it (jitter input) by resolution to keep it same across all res.
	var current_position: vec3<f32> = ray.start.xyz + ray.direction * step_size * jitter(in.position.xy); 
	var step: vec3<f32> = ray.direction * step_size;
 
	var dst: vec4<f32> = vec4<f32>(0.0);

	for (var i: i32 = 0; i < stepsCount; i++)
	{
		// Volume sampling
		var ctVolume: vec4f = textureSample(textMain, samplerLin, current_position);
		var rtVolume: vec4f = textureSample(texAcom, samplerLin, current_position);
		var maskVol: vec4f = textureSample(textMask, samplerNN, current_position);

		// When working with gradients, we need to be careful about in what form we have them from cpu
		var gradient: vec3f = ctVolume.rgb;
		var densityCT: f32 = ctVolume.a * DENSITY_FACTOR_CT;
		var densityRT: f32 = rtVolume.a * DENSITY_FACTOR_RT;

		// Transfer function sampling
		var opacityCT: f32 = textureSample(tfCtOpacity, samplerLin, densityCT).r;
		var colorCT: vec3f = textureSample(tfCtColor, samplerLin, densityCT).rgb;

		var opacityRT: f32 = textureSample(tfRtOpacity, samplerLin, densityRT).r;
		var colorRT: vec3f = textureSample(tfRtColor, samplerLin, densityRT).rgb;

		// Colors
		var color: vec3f = colorCT * (1.0 - opacityRT) + colorRT * opacityRT;
		
		// Opacities
		// var opacity: f32 = GradinetMagnitudeOpacityModulation(opacityCT, gradient);
		var opacity: f32 = IllustrativeContextPreservingOpacity(opacityCT, gradient, wordlCoords, current_position, ray.start, dst.a);

		// Blending
		var src: vec4<f32> = vec4f(color.r, color.g, color.b, opacity);

		if IsInSampleCoords(current_position) && dst.a < 1.0
		{
			dst = FrontToBackBlend(src, dst); 
		}

		// Advance ray
		current_position = current_position + step;
		wordlCoords = wordlCoords + step;
	}

	return dst;
}

