const PI : f32 = 3.1415926536;
const DISTANCE : f32 = 4000;
const SCALE : f32 = 50000;

struct SceneData {
    proj: mat4x4f,
    view: mat4x4f,
    col: vec4f,
    camPos: vec3f,
    pad1: f32,
    camView: vec3f,
    pad2: f32,
    time: f32,
    fade: f32,
};



struct VertexInput {
    @builtin(vertex_index) idx: u32,
};

struct VertexOutput {
    @builtin(position) pos: vec4f,
    @location(0) wPos: vec4f,
}

@group(0) @binding(0) var<uniform> scene: SceneData;
@group(0) @binding(1) var skyboxTexture: texture_cube<f32>;
@group(0) @binding(2) var skyboxSampler: sampler;


@vertex
fn vs_main(in: VertexInput) -> VertexOutput
{
    var out: VertexOutput;

    var cv: vec3f = normalize(scene.camView);
    var a: f32 = PI * 2.0f * f32(in.idx) / 3.0f;
    var sideways: vec3f = normalize(cross(cv, vec3(0, 1, 0)));
    var up: vec3f = cross(cv, sideways);
    out.wPos = vec4(scene.camPos + cv * DISTANCE + up * SCALE * cos(a) + sideways * SCALE * sin(a), 1);
    out.pos = scene.proj * scene.view * out.wPos;

    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f
{
    var cubemapVec = scene.camPos - in.wPos.xyz;
    return textureSample(skyboxTexture, skyboxSampler, cubemapVec) * scene.fade;
}
