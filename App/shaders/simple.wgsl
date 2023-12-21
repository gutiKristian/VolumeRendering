struct Fragment
{
	@builtin(position) position: vec4f,
	@location(0) pos: vec4f,
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

@group(0) @binding(0) var<uniform> camera: CameraData;
@group(0) @binding(1) var<uniform> camera_pos: vec3f;

@group(1) @binding(0) var tex: texture_3d<f32>;
@group(1) @binding(1) var tex_ray_start: texture_2d<f32>;
@group(1) @binding(2) var tex_ray_end: texture_2d<f32>;
@group(1) @binding(3) var texture_sampler: sampler;

@group(2) @binding(0) var<uniform> fragment_mode: i32;

@vertex
fn vs_main(@builtin(vertex_index) v_id: u32, @location(0) vertex_coord: vec3f, @location(1) tex_coord: vec3f) -> Fragment {
	
	let out_position: vec4f = camera.projection * camera.view * camera.model * vec4f(vertex_coord, 1.0);
	
	var vs_out: Fragment;
	
	vs_out.position = out_position;
	vs_out.pos = out_position;
	vs_out.tex_coord = tex_coord;

	return vs_out;
}

fn view_to_world(pixel: vec2<f32>) -> vec3<f32>
{
	//TODO: pass width and height as uniforms or whatever
	var coordNdc: vec2f =  vec2f((pixel.x / 800.0), (pixel.y / 600.0)) * 2.0 - 1.0;
	var temp: vec4<f32> = camera.projection_inverse * vec4f(coordNdc.x, coordNdc.y, 1, 1);
	var coordWorld: vec4f = camera.view_inverse * (temp / temp.w);
	return coordWorld.xyz;
}

// fn rayAABBIntersect(direction: vec3<f32>) -> f32 {
//     var tNear: vec3<f32> = (vec3(-1.0, -1.0, -1.0) - camera_pos) / direction;
//     var tFar: vec3<f32> = (vec3(1.0, 1.0, 1.0) - camera_pos) / direction;

//     // Swap values if necessary to ensure tNear is the minimum
//     tNear = min(tNear, tFar);
//     tFar = max(tNear, tFar);

//     // Find the maximum t value among the components
//     var t1: f32 = max(tNear.x, max(tNear.y, tNear.z));
//     var t2: f32 = min(tFar.x, min(tFar.y, tFar.z));

//     // Check if the intervals are valid
//     if (t1 > t2 || t2 < 0.0) {
//         // No intersection
//         return -1.0;
//     }

//     // The ray intersects the AABB; t1 gives the entry point
//     return t1;
// }


@fragment
fn fs_main(in: Fragment) -> @location(0) vec4<f32>
{

	// var ndc: vec4f = vec4f(
	//     (in.position.x / 800.0 - 0.5) * 2.0,
	//     (in.position.y / 600.0 - 0.5) * 2.0,
	//     (in.position.z - 0.5) * 2.0,
	//     1.0);

	// // Convert NDC throuch inverse clip coordinates to view coordinates
	// var clip: vec4f =  (camera.view_inverse * camera.projection_inverse) * ndc;
	// var vertex: vec3f = (clip / clip.w).xyz;

	// var coordWorld: vec3<f32> = view_to_world(in.position.xy);

	var texC: vec2f = in.pos.xy / in.pos.w;
	texC.x =  0.5*texC.x + 0.5;
	texC.y = -0.5*texC.y + 0.5;

	var ray_start: vec4<f32> = textureSample(tex_ray_start, texture_sampler, texC);
	var ray_end: vec4<f32> = textureSample(tex_ray_end, texture_sampler, texC);

	// var ray_start: vec4<f32> = textureLoad(tex_ray_start, vec2<i32>(i32(in.position.x), i32(in.position.y)), 0);
	// var ray_end: vec4<f32> = textureLoad(tex_ray_end, vec2<i32>(i32(in.position.x), i32(in.position.y)), 0);

	var ray_direction: vec3<f32> = normalize(ray_end.xyz - ray_start.xyz);

	switch fragment_mode {
	  case 1: {
		return vec4<f32>(ray_direction, 1.0);
	  }
	  case 2: {
		return vec4<f32>(ray_start.xyz, 1.0);
	  }
	  case 3: {
		return vec4<f32>(ray_end.xyz, 1.0);
	  }
	  case 4: {
		return vec4<f32>(texC, 0.0, 1.0);
	  }
	  default: {
	  }
	}


	let scaler: f32 = 1 / 4095.0;

	// Position on the cubes surface in uvw format <[0,0,0], [1,1,1]>
	var current_position: vec3<f32> = ray_start.xyz;

	// Iteration params
	var step_size: f32 = 0.01;
	var step: vec3<f32> = ray_direction * step_size;
	var iterations: i32 = 200;

	var final_value: f32 = 0.0;


	var dst: vec4<f32> = vec4<f32>(0.0);
	var src: vec4<f32> = vec4<f32>(0.0);
 
	var value: f32 = 0.0;
 
	for (var i: i32 = 0; i < iterations; i++)
	{
		value = textureSample(tex, texture_sampler, current_position).r * scaler;
		if dst.a <= 0.95
		{     
			src = vec4<f32>(value);
			src.a *= 0.5; //reduce the alpha to have a more transparent result 
			
			//Front to back blending
			// dst.rgb = dst.rgb + (1 - dst.a) * src.a * src.rgb
			// dst.a   = dst.a   + (1 - dst.a) * src.a    
			src.r *= src.a;
			src.g *= src.a; 
			src.b *= src.a; 
			dst = (1.0 - dst.a) * src + dst;
		}

		// Advance ray
		current_position = current_position + step;
	}

	return dst;
}
