#version 330 core

layout (location = 0) out vec4 GBuffer0;

// layout
//      x   y   z   w
// 0    [normal]    0

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Tangent;
} fs_in;

void main()
{
	GBuffer0 = abs(vec4(fs_in.Normal, 0));
}