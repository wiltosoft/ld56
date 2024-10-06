struct VertexOutput {
    @builtin(position) pos: vec4f,
    @location(0) col: vec4f,
    @location(1) uv: vec2f,
};

struct VertexInput {
    @location(0) pos: vec3f,
    @location(1) norm: vec3f,
    @location(2) col: vec4f,
};

@group(0) @binding(3) var fontTexture: texture_2d<f32>;
@group(0) @binding(2) var fontSampler: sampler;


@vertex
fn vs_main(in: VertexInput) -> VertexOutput
{
    var out: VertexOutput;

    out.pos = vec4(in.pos, 1);
    out.col = in.col;
    out.uv = in.norm.xy;

    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f
{
    return vec4f(in.col.rgb, textureSample(fontTexture, fontSampler, in.uv).r * in.col.a);
}


