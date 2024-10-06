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


struct MaterialData {
    col: vec4f,
    emission: vec3f,
    pad1: f32,
    roughness: f32,
    metalness: f32,
}

struct InstanceData {
    @location(3) modelA: vec4f,
    @location(4) modelB: vec4f,
    @location(5) modelC: vec4f,
    @location(6) modelD: vec4f,
    @location(7) col: vec4f,
}

struct VertexOutput {
    @builtin(position) pos: vec4f,
    @location(0) norm: vec3f,
    @location(1) col: vec4f,
    @location(2) wPos: vec4f,

    @location(3) modelA: vec3f,
    @location(4) modelB: vec3f,
    @location(5) modelC: vec3f,
};

struct VertexInput {
    @location(0) pos: vec3f,
    @location(1) norm: vec3f,
    @location(2) col: vec4f,
};

@group(0) @binding(0) var<uniform> scene: SceneData;
@group(0) @binding(1) var skyboxTexture: texture_cube<f32>;
@group(0) @binding(2) var skyboxSampler: sampler;
@group(1) @binding(0) var<uniform> material: MaterialData;


@vertex
fn vs_main(in: VertexInput, instance: InstanceData) -> VertexOutput
{
    var out: VertexOutput;
    let model = mat4x4(instance.modelA, instance.modelB, instance.modelC, instance.modelD);

    out.wPos = model * vec4(in.pos, 1.0);
    out.pos = scene.proj * scene.view * out.wPos;
    out.col = material.col.a * (in.col * vec4(material.col.rgb, 1)) + (1 - material.col.a) * instance.col;

    out.modelA = instance.modelA.xyz;
    out.modelB = instance.modelB.xyz;
    out.modelC = instance.modelC.xyz;
    out.norm = normalize(mat3x3(out.modelA, out.modelB, out.modelC) * in.norm);

    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f
{
    var lightCol: vec3f = vec3f(1, 1, 1);
    var lightVec: vec3f = vec3f(-1, 0, 0);
    var albedo: vec3f = in.col.rgb;
    var roughness: f32 = material.roughness;
    var metalness: f32 = material.metalness;
    var occlusion: f32 = 1.0;
//
    var viewVec: vec3f = normalize(scene.camPos - in.wPos.xyz);
    var N: vec3f = normalize(in.norm);
    var NdotV: f32 = max(dot(N, viewVec), 0.0);
    var reflectVec: vec3f = 2.0 * NdotV * N - viewVec;
    var F0: vec3f = lerp(vec3(0.04), albedo, metalness);

    var halfVec: vec3f = normalize(lightVec + viewVec);
    var F: vec3f = fresnelRough(F0, NdotV, roughness);
    var diffuseDirectTerm: vec3f = albedo * (1 - F) * (1 - metalness);

    var directRadiance: vec3f = lightCol * saturate(dot(N, lightVec));
    var brdfDirectOutput : vec3f = diffuseDirectTerm * directRadiance;

    var specularIrradiance: vec3f = F0 * 2 * textureSample(skyboxTexture, skyboxSampler, -reflectVec).rgb * occlusion;
//    var ambient: vec3f = (kd * albedo * 0.2 + specularIrradiance * F0);
//    let dp = 0.1 + 0.45 * (dot(N.xyz, lightVec) + 1);
//    return vec4f(gammaCorrection(brdfDirectOutput + ambientDiffuse /* + material.emission + specularIrradiance*/).rgb, in.col.a);
//

    return vec4((brdfDirectOutput + material.emission + specularIrradiance) * scene.fade, in.col.a);

//    let dp = 0.1 + 0.45 * (dot(N.xyz, lightVec) + 1);
//    return vec4f(gammaCorrection(dp * in.col.rgb), in.col.a);
}



fn lerp(a: vec3f, b: vec3f, d: f32) -> vec3f
{
    return a + (b - a) * d;
}

fn gammaCorrection(v: vec3f) -> vec3f
{
    return pow(v, vec3f(1.0 / 2.2));
}

fn sRGB2Lin(col: vec3f) -> vec3f
{
    return pow(col, vec3f(2.2, 2.2, 2.2));
}

fn fresnelRough(F0: vec3f, NdotV: f32, roughness: f32) -> vec3f
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * fPow5(NdotV);
}

fn fPow5(v: f32) -> f32
{
    return pow(1 - v, 5);
}
