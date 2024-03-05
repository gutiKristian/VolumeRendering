struct Fragment
{
	@builtin(position) position: vec4f,
	@location(0) world_coord: vec4f,
	@location(1) tex_coord: vec3f
}

struct CameraData
{
	model: mat4x4<f32>,
	view: mat4x4<f32>,
	projection: mat4x4<f32>,
	view_inverse: mat4x4<f32>,
	projection_inverse: mat4x4<f32>
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

@group(0) @binding(0) var<uniform> camera: CameraData;
@group(0) @binding(1) var<uniform> camera_pos: vec3f;

@group(1) @binding(0) var tex: texture_3d<f32>;
@group(1) @binding(1) var tex_ray_start: texture_2d<f32>;
@group(1) @binding(2) var tex_ray_end: texture_2d<f32>;
@group(1) @binding(3) var texture_sampler: sampler;
@group(1) @binding(4) var tex_tf: texture_1d<f32>;
@group(1) @binding(5) var tex_sampler_nn: sampler;
@group(1) @binding(6) var texAcom: texture_3d<f32>;
@group(1) @binding(7) var texTfColor: texture_1d<f32>;


@group(2) @binding(0) var<uniform> fragment_mode: i32;
@group(2) @binding(1) var<uniform> steps_count: i32;

@group(3) @binding(0) var<uniform> light: LightData;

@vertex
fn vs_main(@builtin(vertex_index) v_id: u32, @location(0) vertex_coord: vec3f, @location(1) tex_coord: vec3f) -> Fragment {
	
	let out_position: vec4f = camera.projection * camera.view * camera.model * vec4f(vertex_coord, 1.0);
	
	var vs_out: Fragment;
	
	vs_out.position = out_position;
	// passing world coordinates
	vs_out.world_coord = camera.model * vec4f(vertex_coord, 1.0);
	vs_out.tex_coord = tex_coord;

	return vs_out;
}


/*
* Checks whether the position is within out bbox basically,
* but our bbox coordinates are basically 3D tex. coordinates
*/
fn is_in_sample_coords(position: vec3<f32>) -> bool
{
	var b_min: vec3<f32> = vec3<f32>(0.0, 0.0, 0.0);
	var b_max: vec3<f32> = vec3<f32>(1.0, 1.0, 1.0);

	return	position.x >= b_min.x && position.x <= b_max.x &&
			position.y >= b_min.y && position.y <= b_max.y &&
			position.z >= b_min.z && position.z <= b_max.z;
}

fn setup_ray(tex_pos: vec2<i32>) -> Ray
{
	var ray: Ray;
	// Ray setup
	ray.start = textureLoad(tex_ray_start, tex_pos, 0).xyz;
	ray.end = textureLoad(tex_ray_end, tex_pos, 0).xyz;
	ray.direction = normalize(ray.end.xyz - ray.start.xyz);
	ray.length = length(ray.end.xyz - ray.start.xyz);
	return ray;
}

fn blend(src: vec4<f32>, dst: vec4<f32>) -> vec4<f32>
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
	N - unit normal vector
	L- unit vector pointing from sampled point to the light
	V - unit view vector from point to camera
*/
fn blinnPhong(N: vec3f, worldPosition: vec3f) -> vec3f
{
	var L: vec3f = normalize(light.position - worldPosition);
	var V: vec3f = normalize(camera_pos - worldPosition);
	var R: vec3f = 2 * (N * L)* N - L;
	var H: vec3f = normalize(V + L); 
	return light.diffuse * max(dot(N, L), 0.0) * 1.2;// + light.specular * pow(max(dot(N, H), 0.0), 3);
}

// Either upload max value for dataset as uniform or upload already normalised data (had some gradient troubles)
const DENSITY_FACTOR_CT = 1/4095.0; // Divide by -> max possible (2900 something), 2^(used bits) - 1 4095, 2^dicom param allocated bits 2^16
const DENSITY_FACTOR_RT = 1/32767.0;
// dividing by max value from data we are stretching them more for instance: max val is 2900:  2900 / 2900 -> 1 
// and 2900/4095 -> 0.7 so using max value we get finer mapping and higher res
// as we stretch 0-2900 to [0,1] instead of 0-4095 to [0,1]

@fragment
fn fs_main(in: Fragment) -> @location(0) vec4<f32>
{

	// If we would like to sample the texture with a sampler, this transforms the coordinates in ndc to texture
	// and as we rendered the cube to the texture of size of screen this gives us the coords, Y IS FLIPPED
	var texC: vec2f = in.world_coord.xy / in.world_coord.w;
	texC.x =  0.5*texC.x + 0.5;
	texC.y = -0.5*texC.y + 0.5;

	var wordlCoords: vec3f = in.world_coord.xyz;
	// Ray setup
	let ray: Ray = setup_ray(vec2<i32>(i32(in.position.x), i32(in.position.y)));

	switch fragment_mode {
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

	for (var i: i32 = 0; i < steps_count; i++)
	{
		var ctVolume: vec4f = textureSample(tex, texture_sampler, current_position);
		var rtVolume: vec4f = textureSample(texAcom, texture_sampler, current_position);
		
		// for now, the values of gradient and density are untouched on cpu side
		// var gradient: vec3f = normalize(ctVolume.rgb);
		// var gradient: vec3f = computeGradient(current_position, step_size);
		var densityCT: f32 = ctVolume.a * DENSITY_FACTOR_CT;
		var densityRT: f32 = rtVolume.a * DENSITY_FACTOR_RT;

		var opacity: f32 = textureSample(tex_tf, texture_sampler, densityCT).r;
		var color: vec3f = textureSample(texTfColor, texture_sampler, densityRT).rgb;

		// Blinn-Phong
		// var lightColor = blinnPhong(gradient, wordlCoords);
		// color = light.ambient + vec3(0.1) + lightColor * color;


		// Blending
		var src: vec4<f32> = vec4f(color.r, color.g, color.b, opacity);

		if is_in_sample_coords(current_position) && dst.a < 1.0
		{
			dst = blend(src, dst); 
		}

		// Advance ray
		current_position = current_position + step;
		wordlCoords = wordlCoords + step;
	}

	return dst;
}


//  ------------------------------- UNUSED -------------------------------

fn computeGradient(position: vec3<f32>, step: f32) -> vec3f
{
	var result = vec3f(0.0, 0.0, 0.0);
	var dirs = array<vec3f, 3>(vec3f(1.0, 0.0, 0.0), vec3f(0.0, 1.0, 0.0), vec3f(0.0, 0.0, 1.0));
	result.x = textureSample(tex, texture_sampler, position + dirs[0] * step).a - textureSample(tex, texture_sampler, position - dirs[0] * step).a;
	result.y = textureSample(tex, texture_sampler, position + dirs[1] * step).a - textureSample(tex, texture_sampler, position - dirs[1] * step).a;
	result.z = textureSample(tex, texture_sampler, position + dirs[2] * step).a - textureSample(tex, texture_sampler, position - dirs[2] * step).a;
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
fn view_to_world(pixel: vec2<f32>) -> vec3<f32>
{
	// required to pass width and height as uniforms or whatever
	var coordNdc: vec2f =  vec2f((pixel.x / 1280.0), (pixel.y / 720.0)) * 2.0 - 1.0;
	var temp: vec4<f32> = camera.projection_inverse * vec4f(coordNdc.x, coordNdc.y, 1, 1);
	var coordWorld: vec4f = camera.view_inverse * (temp / temp.w);
	return coordWorld.xyz;
}


/*
* Fixed sample size
* Based on step_size, it returns number of samples/steps on a ray
* PROBLEM WITH NON UNIFORM FLOW...
*/
fn get_number_of_samples(ray_length: f32, step_size: f32) -> i32
{
	return i32(ray_length / step_size);
}

/*
* Based on number of samples, it returns the size of the step to fit desired number of samples/steps
*/
fn get_step_size(ray_length: f32, samples: i32) -> f32
{
	return ray_length / f32(samples);
}