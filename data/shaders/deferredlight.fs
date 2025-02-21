#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

// layout
//      x   y   z   w
// 0    [albedo]  roughness
// 1    [normal]  metalness
// 2    [ pos  ]    0
// 3    0   0   0   0

uniform sampler2D gbuffer0;
uniform sampler2D gbuffer1;
uniform sampler2D gbuffer2;
uniform sampler2D gbuffer3;

uniform vec3 pointlight_pos;
uniform vec3 pointlight_radiance;

void main() {
	vec4 data0 = texture(gbuffer0, TexCoords);
	vec4 data1 = texture(gbuffer1, TexCoords);
	vec4 data2 = texture(gbuffer2, TexCoords);
	vec4 data3 = texture(gbuffer3, TexCoords);
	
	vec3 albedo = data0.xyz;
	float roughness = data0.w;
	vec3 normal = data1.xyz;
	float metalness = data1.w;
	vec3 pos = data2.xyz;
	
	// point light
	vec3 fragTolight = pointlight_pos - pos; // frag to light
	float dist2 = dot(fragTolight, fragTolight);
	float dist = sqrt(dist2);
	vec3 L = fragTolight / dist;
	float cos_theta = dot(normal, L);
	vec3 Lo = albedo * pointlight_radiance * max(cos_theta, 0) / dist2;
	
	FragColor = vec4(Lo, roughness * metalness);
}