#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D inputTexture;

void main()
{             
    FragColor = vec4(texture(inputTexture, TexCoords).rgb, 1.0);
}