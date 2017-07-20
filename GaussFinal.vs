#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec4 IPCoords;
out vec4 FragPosLightSpace;
out vec4 FragPosViewSpace;
out vec3 inNormal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
	vec3 temp = vec3(model * vec4(aPos, 1.0));
	FragPosLightSpace = lightSpaceMatrix * vec4(temp, 1.0);
	IPCoords = projection * view * model * vec4(aPos, 1.0);
	FragPosViewSpace = projection * view * model * vec4(aPos, 1.0);
    inNormal = transpose(inverse(mat3(model))) * aNormal;
	FragPos = vec3(model * vec4(aPos, 1.0));
	TexCoords = aTexCoords;
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}