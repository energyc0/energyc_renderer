#version 450

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 offset;
layout(location = 2) in float radius;

layout(location = 0) out vec4 out_color;

void main(){
    if(radius * radius < dot(offset,offset)){
        discard;
    }
    out_color = vec4(color,1.0);
}