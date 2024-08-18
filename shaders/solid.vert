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

layout(set = 1, binding = 1) readonly buffer Transform{
    mat4 model[GROUP_MODEL_LIMIT];
}transform;


layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec3 frag_color;

void main(){
    frag_color = colors[gl_InstanceIndex % 3];
    gl_Position = ubo.perspective * ubo.view * transform.model[gl_InstanceIndex] * vec4(pos, 1.0);
    //gl_Position = ubo.perspective * ubo.view  * vec4(pos, 1.0);
}