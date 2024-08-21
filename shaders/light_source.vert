#version 450

#define POINT_LIGHT_LIMIT 10

struct PointLight{
    vec4 pos;
    vec3 color;
};

layout(set = 0, binding = 0) uniform Global_UBO{
    mat4 view;
    mat4 projection;
}global_ubo;

layout(set = 1, binding = 1) uniform PointLight_UBO{
    PointLight lights[POINT_LIGHT_LIMIT];
} light_ubo;

const vec2 offsets[6] = {
    vec2(-1.0,-1.0),
    vec2(-1.0,1.0),
    vec2(1.0,-1.0),
    vec2(1.0,-1.0),
    vec2(-1.0,1.0),
    vec2(1.0,1.0)
};

layout(location = 0) out vec3 color;
layout(location = 1) out vec2 offset;
layout(location = 2) out float radius;

void main(){
    radius = light_ubo.lights[gl_InstanceIndex].pos.w;
    color = light_ubo.lights[gl_InstanceIndex].color;
    offset = radius * offsets[gl_VertexIndex];

    gl_Position = global_ubo.projection * 
    (global_ubo.view * vec4(light_ubo.lights[gl_InstanceIndex].pos.xyz, 1.0) + vec4(offset,0.0, 0.0));
}