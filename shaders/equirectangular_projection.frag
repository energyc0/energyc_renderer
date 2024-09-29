#version 450

#define PI 3.1415926535897930
#define half_PI 1.5707963267948965

layout(binding = 0) uniform sampler2D equirectangular_map;
layout(location = 0) in vec3 pos;
layout(location = 0) out vec4 out_color;

vec2 get_spherical_coordinates(vec3 position){
    vec2 uv = vec2(atan(position.z, position.x), asin(position.y));
    uv = vec2(0.5) + uv / vec2(PI, half_PI);
    return uv;
}

void main(){
    vec2 uv = get_spherical_coordinates(normalize(pos));
    vec3 color = texture(equirectangular_map, uv).rgb;
    out_color = vec4(color, 1.0);
}
