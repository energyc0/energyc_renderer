#version 450

const vec2 vertices[] = {
    vec2(-1.0,-1.0),
    vec2(-1.0,1.0),
    vec2(1.0,-1.0),
    vec2(1.0,-1.0),
    vec2(-1.0,1.0),
    vec2(1.0,1.0)
};

layout(location = 0) out vec2 uv;

void main(){
    uv = (vertices[gl_VertexIndex] + 1.0) * 0.5;
    gl_Position = vec4(vertices[gl_VertexIndex], 0.0,1.0);
}