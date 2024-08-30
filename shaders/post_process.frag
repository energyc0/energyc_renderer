#version 450

const float weights[] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};

layout(input_attachment_index = 0, binding = 0) uniform subpassInput input_color;
layout(binding = 1) uniform sampler2D gauss_blur_color;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_color;

void main(){
    vec3 color = texture(gauss_blur_color, uv).rgb * weights[0];
    vec2 offset = 1.0 / textureSize(gauss_blur_color,0);
    float y = 1.0;
    for(int i = 1; i < 5; i++){
        color += texture(gauss_blur_color, uv + offset * vec2(0.0,y)).rgb * weights[i];
        color += texture(gauss_blur_color, uv - offset * vec2(0.0,y)).rgb * weights[i];
        y++;
    }
    color += subpassLoad(input_color).rgb;
    color = color / (color + vec3(1.0));

    out_color = vec4(color,1.0);
}