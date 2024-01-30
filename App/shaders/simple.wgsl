struct Fragment
{
	@builtin(position) position: vec4f,
	@location(0) raw_pos: vec4f,
	@location(1) tex_coord: vec3f
}

struct CameraData
{
	model: mat4x4<f32>,
	view: mat4x4<f32>,
	projection: mat4x4<f32>,
	view_inverse: mat4x4<f32>,
	projection_inverse: mat4x4<f32>
};

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

@group(2) @binding(0) var<uniform> fragment_mode: i32;
@group(2) @binding(1) var<uniform> steps_count: i32;

@vertex
fn vs_main(@builtin(vertex_index) v_id: u32, @location(0) vertex_coord: vec3f, @location(1) tex_coord: vec3f) -> Fragment {
	
	let out_position: vec4f = camera.projection * camera.view * camera.model * vec4f(vertex_coord, 1.0);
	
	var vs_out: Fragment;
	
	vs_out.position = out_position;
	vs_out.raw_pos = out_position;
	vs_out.tex_coord = tex_coord;

	return vs_out;
}

/*
* Helper for change of base
* For now UNUSED
*/
fn view_to_world(pixel: vec2<f32>) -> vec3<f32>
{
	//! pass width and height as uniforms or whatever
	var coordNdc: vec2f =  vec2f((pixel.x / 1280.0), (pixel.y / 720.0)) * 2.0 - 1.0;
	var temp: vec4<f32> = camera.projection_inverse * vec4f(coordNdc.x, coordNdc.y, 1, 1);
	var coordWorld: vec4f = camera.view_inverse * (temp / temp.w);
	return coordWorld.xyz;
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

/*
* Fixed sample size
* Based on step_size, it returns number of samples/steps on a ray
* PROBLEM WITH NON UNIFORM FLOW...
*/
// fn get_number_of_samples(ray_length: f32, step_size: f32) -> i32
// {
// 	return i32(ray_length / step_size);
// }

/*
* Based on number of samples, it returns the size of the step to fit desired number of samples/steps
*/
fn get_step_size(ray_length: f32, samples: i32) -> f32
{
	return ray_length / f32(samples);
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

@fragment
fn fs_main(in: Fragment) -> @location(0) vec4<f32>
{

	// If we would like to sample the texture with a sampler, this transforms the coordinates in ndc to texture
	// and as we rendered the cube to the texture of size of screen this gives us the coords, Y IS FLIPPED
	var texC: vec2f = in.raw_pos.xy / in.raw_pos.w;
	texC.x =  0.5*texC.x + 0.5;
	texC.y = -0.5*texC.y + 0.5;
	
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
	var step_size: f32 = get_step_size(ray.length, steps_count);

 	// Position on the cubes surface in uvw format <[0,0,0], [1,1,1]>
	var current_position: vec3<f32> = ray.start.xyz;
	var step: vec3<f32> = ray.direction * step_size;

	var dst: vec4<f32> = vec4<f32>(0.0);

	for (var i: i32 = 0; i < steps_count; i++)
	{
		var raw_intensity: f32 = textureSample(tex, texture_sampler, current_position).r;
		var normalized_intensity: f32 = raw_intensity / 4095.0;
		var tf: f32 = textureSample(tex_tf, texture_sampler, normalized_intensity).r;
		
		var src: vec4<f32> = vec4<f32>(tf);

		if is_in_sample_coords(current_position) && dst.a < 1.0
		{
			dst.r = src.a * src.r + (1 - src.a) * dst.a * dst.r;
			dst.g = src.a * src.g + (1 - src.a) * dst.a * dst.g;
			dst.b = src.a * src.b + (1 - src.a) * dst.a * dst.b;
    		
			dst.a = src.a + (1 - src.a) * dst.a;
		}

		// Advance ray
		current_position = current_position + step;
	}

	return dst;
}
