#version 450
#define POINT_LIGHT_LIMIT 10

struct PointLight{
    vec3 pos;
    vec3 color;
};

layout(location = 0) in vec3 frag_pos;
layout(location = 1) in vec3 frag_color;
layout(location = 2) in vec2 frag_uv;
layout(location = 3) in mat3 TBN;

layout(location = 0) out vec4 out_color;

layout(set = 1, binding = 1) uniform PointLight_UBO{
    PointLight lights[POINT_LIGHT_LIMIT];
} light_ubo;

layout(set = 2, binding = 0) uniform sampler2D material[4];
//material
#define ALBEDO 0
#define METALLIC 1
#define ROUGHNESS 2
#define NORMAL 3
//

layout(push_constant) uniform push_data{
    vec3 camera_pos;
} push;

vec3 calculate_lighting(PointLight light, vec3 frag_to_camera, vec3 color, vec3 frag_normal){
    float ambient = 0.1;

    vec3 frag_to_light = normalize(light.pos - frag_pos);
    float diffuse = max(0.0, dot(frag_normal, frag_to_light));

    vec3 half_way_vec = normalize(frag_to_camera + frag_to_light);
    float specular = pow(max(0.0, dot(half_way_vec, frag_normal)), 32);
    vec3 result = (diffuse + ambient + specular) * light.color * color;
    return result;
}

void main(){
    vec3 color = texture(material[ALBEDO], frag_uv).rgb;
    vec3 frag_to_camera = normalize(push.camera_pos - frag_pos);
    vec3 norm = normalize(TBN * (texture(material[NORMAL],frag_uv).xyz * 2.0 - 1.0));
    color = calculate_lighting(light_ubo.lights[0], frag_to_camera, color, norm);
    out_color = vec4(color, 1.0);
}