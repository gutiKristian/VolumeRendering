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

struct LightData
{
	position: vec3<f32>,
	_alignment01: f32,
	ambient: vec3<f32>,
	_alignment02: f32,
	diffuse: vec3<f32>,
	_alignment03: f32
}

// Default bindings
@group(0) @binding(0) var<uniform> camera: CameraData;
@group(0) @binding(1) var<uniform> cameraPosition: vec3f;
@group(0) @binding(2) var samplerLin: sampler;
@group(0) @binding(3) var samplerNN: sampler;
@group(0) @binding(4) var<uniform> fragmentMode: i32;
@group(0) @binding(5) var<uniform> stepsCount: i32;
@group(0) @binding(6) var<uniform> stepsSize: f32;
@group(0) @binding(7) var<uniform> clipX: vec2<f32>;
@group(0) @binding(8) var<uniform> clipY: vec2<f32>;
@group(0) @binding(9) var<uniform> clipZ: vec2<f32>;
@group(0) @binding(10) var<uniform> toggles: vec4<i32>;
@group(0) @binding(11) var texRayEnd: texture_2d<f32>;

// App
@group(1) @binding(0) var textMain: texture_3d<f32>;
@group(1) @binding(1) var tfOpacity: texture_1d<f32>;
@group(1) @binding(2) var tfColor: texture_1d<f32>;
@group(1) @binding(3) var<uniform> light: LightData;

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
* Based on number of samples, it returns the size of the step to fit desired number of samples/steps
*/
fn GetStepSize(rayLength: f32, samples: i32) -> f32
{
	return rayLength / f32(samples);
}

fn CalculateWorldStep(r: Ray, stepSize: f32) -> vec3<f32>
{
	// BBox ratio from uniforms
	var worldStep: vec3<f32> = r.direction * vec3f(stepSize * 1.0, stepSize * 1.0, stepSize * 0.7);
	worldStep.z *= (-1.0); // it is inverted  in the world space
	return worldStep;
}

/*
* Checks whether the position is within out bbox basically,
* but our bbox coordinates are basically 3D texture coordinates
*/
fn IsInSampleCoords(position: vec3<f32>) -> bool
{
	var b_min: vec3<f32> = vec3<f32>(0.0 + clipX.x, 0.0 + clipY.x, 0.0 + clipZ.x);
	var b_max: vec3<f32> = vec3<f32>(1.0 - clipX.y, 1.0 - clipY.y, 1.0 - clipZ.y);

	return	position.x >= b_min.x && position.x <= b_max.x &&
			position.y >= b_min.y && position.y <= b_max.y &&
			position.z >= b_min.z && position.z <= b_max.z;
}

/*
* Sets up the ray for the fragment shader
* @param screenSpaceCoord: screen space coordinates of the fragment, used to sample pre-rendered ray start and end
* @return Ray: ray with start, end, direction and length
*/
fn SetupRay(screenSpaceCoord: vec2<i32>, start: vec3<f32>) -> Ray
{
	var ray: Ray;
	// Ray setup
	ray.start = start;
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
* Calculates Blinn-Phong shading model (no specular)
* @param N: unit normal vector
* @param L: unit vector pointing from sampled point to the light
* @param V: unit view vector from point to camera
*/
fn BlinnPhong(N: vec3f, worldPosition: vec3f) -> vec3<f32>
{
	let kD: f32 = 2.5;
	let kA: f32 = 0.5;
	var L: vec3f = normalize(light.position - worldPosition);
	// var V: vec3f = normalize(cameraPosition - worldPosition);
	// var R: vec3f = 2 * (N * L)* N - L;
	// var H: vec3f = normalize(V + L); 

	return light.diffuse * max(dot(N, L), 0.0) * kD + light.ambient * kA; // length(light.specular * pow(max(dot(N, H), 0.0), 3)) * 1.0 + 0.3;
}



@fragment
fn fs_main(in: Fragment) -> @location(0) vec4<f32>
{

	// If we would like to sample the texture with a sampler, this transforms the coordinates in ndc to texture
	// and as we rendered the cube to the texture of size of screen this gives us the coords, Y IS FLIPPED
	var texC: vec2f = in.worldCoord.xy / in.worldCoord.w;
	texC.x =  0.5*texC.x + 0.5;
	texC.y = -0.5*texC.y + 0.5;

	var wCoords: vec3f = in.worldCoord.xyz;

	// Ray setup
	let ray: Ray = SetupRay(vec2<i32>(i32(in.position.x), i32(in.position.y)), in.textureCoord);

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
	var stepSize: f32 = stepsSize;
	var worldStep: vec3<f32> = CalculateWorldStep(ray, stepSize);

	
	if toggles[0] == 1
	{
		stepSize = GetStepSize(ray.length, stepsCount);
	}

 	// Position on the cubes surface in uvw format <[0,0,0], [1,1,1]>
	var currentPosition: vec3<f32> = ray.start.xyz;

	if toggles[1] == 1
	{
		// apply jitter using screen space coordinates, we could divide it (jitter input) by resolution to keep it same across all res.
		currentPosition = currentPosition + ray.direction * stepSize * jitter(in.position.xy);
	}

	var step: vec3<f32> = ray.direction * stepSize;
	
	// Resulting pixel color
	var dst: vec4<f32> = vec4<f32>(0.0);

	for (var i: i32 = 0; i < stepsCount; i++)
	{
		// Volume sampling
		var volumeSample: vec4f = textureSample(textMain, samplerLin, currentPosition);
		var gradient: vec3<f32> = volumeSample.rgb;
		//gradient = ComputeGradient(currentPosition, stepSize, textMain);

		var density: f32 = volumeSample.a;

		// Transfer function sampling
		var opacity: f32 = textureSample(tfOpacity, samplerLin, density).r;
		var color: vec3f = textureSample(tfColor, samplerLin, density).rgb;
		
		if IsInSampleCoords(currentPosition) && dst.a < 1.0
		{
			color *= BlinnPhong(normalize(gradient), wCoords);
			
			// Gradient based opacity modulation
			// opacity *= length(gradient);
			
			// Blending
			dst = FrontToBackBlend(vec4f(color.r, color.g, color.b, opacity), dst); 
		}

		// Advance ray
		currentPosition = currentPosition + step;
		wCoords = wCoords + worldStep;
	}

	return dst;
}

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