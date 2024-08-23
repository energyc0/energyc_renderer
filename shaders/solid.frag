#version 450

#define POINT_LIGHT_LIMIT 10
#define PI 3.1415926535

struct PointLight{
    vec4 pos;
    vec3 color;
};

layout(location = 0) in vec3 frag_pos;
layout(location = 1) in vec3 frag_color;
layout(location = 2) in vec2 frag_uv;
layout(location = 3) in vec3 frag_normal;
layout(location = 4) in mat3 TBN;

layout(location = 0) out vec4 out_color;

layout(set = 1, binding = 1) uniform PointLight_UBO{
    PointLight lights[POINT_LIGHT_LIMIT];
} light_ubo;

layout(set = 2, binding = 0) uniform sampler2D material[4];

layout(set = 2, binding = 1) uniform Material_const{
    vec3 albedo;
    float metalness;
    float roughness;
    bool has_normal;
}ubo_material;

//material
#define ALBEDO 0
#define METALLIC 1
#define ROUGHNESS 2
#define NORMAL 3
//

layout(push_constant) uniform push_data{
    vec3 camera_pos;
} push;

vec3 Fresnel_function(vec3 albedo,vec3 F0, float metalness, float cos_view_halfway){
    return F0 + (1.0 - F0) * pow((1.0 - cos_view_halfway),5.0);
}

// fspecular
vec3 Cook_Torrance_function(float D, vec3 F, float G, vec3 V, vec3 N, vec3 L){
    return              (D * F * G) /
    (4.0 * max(dot(V, N),0.00001) * max(dot(L, N),0.00001));
}

//Normal distribution function
float GGX_Trowbridge_Reitz(float roughness, float cos_norm_halfway){
    float alpha_sqr = roughness * roughness * roughness * roughness;
    float denom = cos_norm_halfway * cos_norm_halfway * (alpha_sqr - 1.0) + 1.0;
    return alpha_sqr/
    max((PI * denom * denom), 0.00001);
}

float Geometry_Schlick_Beckman(vec3 N, vec3 X, float k){
    return max(dot(N,X),0.0)/
    max((max(dot(N,X),0.0) * (1.0 - k) + k),0.00001);
}

float Geometry_Smith(vec3 N, vec3 L, vec3 V, float roughness){
    float k = roughness / 2.0;
    return Geometry_Schlick_Beckman(N,L,k) * Geometry_Schlick_Beckman(N,V,k);
}

vec3 calculate_lighting(PointLight light, vec3 V, vec3 albedo, vec3 N, float metalness, float roughness, vec3 F0){ 
    vec3 L = light.pos.xyz - frag_pos;
    float dist = length(L);
    float attenuation = 1.0;
    L = normalize(L);
    vec3 H = normalize(V + L);

    // BRDF = Kd * fdiffuse + Ks * fspecular

    vec3 Fresnel = Fresnel_function(albedo, F0, metalness, max(0.0, dot(V, H)));

    vec3 Kd = (1.0 - Fresnel) * (1.0 - metalness); // metals do not have diffuse
    // Lambert function = (color / PI) * dot(L, N)
    vec3 fdiffuse = albedo / PI;

    float D = GGX_Trowbridge_Reitz(roughness,max(dot(N, H), 0.0));
    float G = Geometry_Smith(N, L, V, roughness);

    vec3 fspecular = Cook_Torrance_function(D, Fresnel,G , V,N,L);

    vec3 result = (Kd * fdiffuse + fspecular) * attenuation * light.color * max(dot(N,L),0.0);   // we have already multiplied by Ks in Cook-Torrance function
    return result;
}

void main(){
    vec3 albedo;
    if(ubo_material.albedo == vec3(-1.0)){
         albedo = texture(material[ALBEDO], frag_uv).rgb;
    }else{
        albedo = ubo_material.albedo;
    }

    vec3 normal;
    if(ubo_material.has_normal){
        normal = normalize(TBN * (texture(material[NORMAL],frag_uv).xyz * 2.0 - 1.0));
    }else{
        normal = frag_normal;
    }

    float metalness;
    if(ubo_material.metalness == -1.0){
        metalness = texture(material[METALLIC], frag_uv).r;
    }else{
        metalness = ubo_material.metalness;
    }

    float roughness;
    if(ubo_material.roughness == -1.0){
        roughness = texture(material[ROUGHNESS],frag_uv).r;
    }else{
        roughness = ubo_material.roughness;
    }
    vec3 frag_to_camera = normalize(push.camera_pos - frag_pos);

    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    vec3 color = vec3(0.01) * albedo;

    color += calculate_lighting(light_ubo.lights[0], frag_to_camera, albedo, normal, metalness, roughness, F0);

    //color = color / (color + vec3(1.0));
    out_color = vec4(color, 1.0);
}