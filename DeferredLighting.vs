#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

out VS_OUT {
    vec3 FragPos;
	vec4 FragPosViewSpace;
    vec3 Normal;
    vec2 TexCoords;
	flat int numLights;
} vs_out;

uniform int anumLights;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    vs_out.TexCoords = aTexCoords;
	vs_out.numLights = anumLights;
	vs_out.FragPosViewSpace = projection * view * model * vec4(aPos, 1.0);
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}