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
layout(location = 4) in vec3 tangent;

layout(location = 0) out vec3 frag_pos;
layout(location = 1) out vec3 frag_color;
layout(location = 2) out vec2 frag_uv;
layout(location = 3) out vec3 frag_normal;
layout(location = 4) out mat3 TBN;

void main(){
    mat3 model_inverse = inverse(transpose(mat3(transform.model[gl_InstanceIndex])));
    frag_color = color;
    frag_pos = vec3(transform.model[gl_InstanceIndex] * vec4(pos,1.0));
    frag_uv = uv;

    frag_normal = normalize(mat3(transform.model[gl_InstanceIndex]) * normal);
    vec3 T = normalize(mat3(transform.model[gl_InstanceIndex]) * tangent);
    vec3 B = cross(T,frag_normal);
    TBN = mat3(T,B,frag_normal);
    
    gl_Position = ubo.perspective * ubo.view * vec4(frag_pos, 1.0);
}