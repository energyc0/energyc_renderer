#version 450

layout(location = 0) in vec2 uv;
layout(binding = 0) uniform sampler2D image;

layout(location = 0) out vec4 out_color;

const float weights[] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};

void main(){
    vec2 pixel_size = 1.0 / textureSize(image,0);
    vec3 color = texture(image, uv).rgb * weights[0];
    float x = 1.0;
    for(int i = 1; i < 5; i++){
        color += texture(image, uv + vec2(x, 0.0) * pixel_size).rgb * weights[i];
        color += texture(image, uv - vec2(x, 0.0) * pixel_size).rgb * weights[i];
        x++;
    }
    out_color = vec4(color, 1.0);
}