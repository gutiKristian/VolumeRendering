struct Fragment
{
	@builtin(position) position: vec4f,
	@location(0) color: vec4f
}


@vertex
fn vs_main(@builtin(vertex_index) v_id: u32) -> Fragment
{

    let pos = array<vec2<f32>, 6>(
    vec2<f32>(-1.0, -1.0),
    vec2<f32>(1.0, -1.0),
    vec2<f32>(1.0, 1.0),

    vec2<f32>(-1.0, -1.0),
    vec2<f32>(1.0, 1.0),
    vec2<f32>(-1.0, 1.0)
    );

    // let col = array<vec4<f32>, 6>(
    // vec4<f32>(0.0, 0.0, 0.0, 1.0),
    // vec4<f32>(0.0, 0.0, 0.0, 1.0),
    // vec4<f32>(1.0, 1.0, 1.0, 1.0),

    // vec4<f32>(0.0, 0.0, 0.0, 1.0),
    // vec4<f32>(1.0, 1.0, 1.0, 1.0),
    // vec4<f32>(1.0, 1.0, 1.0, 1.0),
    // );
    

    let col = array<vec4<f32>, 6>(
    vec4<f32>(1.0, 1.0, 1.0, 1.0),
    vec4<f32>(1.0, 1.0, 1.0, 1.0),
    vec4<f32>(1.0, 1.0, 1.0, 1.0),

    vec4<f32>(1.0, 1.0, 1.0, 1.0),
    vec4<f32>(1.0, 1.0, 1.0, 1.0),
    vec4<f32>(1.0, 1.0, 1.0, 1.0),
    );

    var vs_out: Fragment;
    vs_out.position = vec4<f32>(pos[v_id], 0.0, 1.0);
    vs_out.color = col[v_id];
    return vs_out;
}

@fragment
fn fs_main(in: Fragment) -> @location(0) vec4<f32>
{
	return in.color;
}
