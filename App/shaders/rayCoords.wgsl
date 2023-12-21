struct Fragment
{
	@builtin(position) position: vec4f,
	@location(0) tex_coord: vec3f
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

@vertex
fn vs_main(@builtin(vertex_index) v_id: u32, @location(0) vertex_coord: vec3f, @location(1) tex_coord: vec3f) -> Fragment {
	
	var vs_out: Fragment;
	let out_position: vec4f = camera.projection * camera.view * camera.model * vec4f(vertex_coord, 1.0);
	vs_out.position = out_position;
 	vs_out.tex_coord = tex_coord;
	return vs_out;
}

@fragment
fn fs_main(in: Fragment) -> @location(0) vec4<f32>
{
	return vec4f(in.tex_coord, 1.0);
}
