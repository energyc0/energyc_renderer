#version 450

const vec2 vertices[] = {
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5),
    vec2(0.0, 0.0)
};

const vec3 colors[] = {
    vec3(1.0,0.0,0.0),
    vec3(0.0,1.0,0.0),
    vec3(0.0,0.0,1.0)
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec3 frag_color;

void main(){
    frag_color = colors[gl_VertexIndex];
    gl_Position = vec4(vertices[gl_VertexIndex], 0.0, 1.0);
}