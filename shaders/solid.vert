#version 450

#define GROUP_MODEL_LIMIT 30

const vec3 colors[] = {
    vec3(1.0,0.0,0.0),
    vec3(0.0,1.0,0.0),
    vec3(0.0,0.0,1.0)
};

layout(set = 0, binding = 0) uniform global_UBO{
    mat4 view;
    mat4 perspective;
}ubo;

layout(set = 1, binding = 0) readonly buffer Transform{
    mat4 model[GROUP_MODEL_LIMIT];
}transform;


layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 frag_pos;
layout(location = 1) out vec3 frag_color;
layout(location = 2) out vec3 frag_normal;
layout(location = 3) out vec2 frag_uv;

void main(){
    frag_color = color;
    frag_pos = vec3(transform.model[gl_InstanceIndex] * vec4(pos,1.0));
    frag_normal = normalize(inverse(transpose(mat3(transform.model[gl_InstanceIndex]))) * normal);
    frag_uv = uv;
    gl_Position = ubo.perspective * ubo.view * vec4(frag_pos, 1.0);
}