#version 410 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;
uniform float lightSize;
uniform vec2 dir;


void main()
{   
	  float sample_step = lightSize * 0.065;

	  float offsets[10]; 
  
    offsets[0] = 5.0 * sample_step;
    offsets[1] = 4.0 * sample_step;
    offsets[2] = 3.0 * sample_step;
    offsets[3] = 2.0 * sample_step;
    offsets[4] = sample_step;
	offsets[5] = -5.0 * sample_step;
    offsets[6] = -4.0 * sample_step;
    offsets[7] = -3.0 * sample_step;
    offsets[8] = -2.0 * sample_step;
    offsets[9] = -sample_step;

	
	float minz = texture( depthMap, TexCoords ).r;  //sample at the center	

	for( int i = 0; i < 10; ++i )
	{
		minz = min( minz, texture( depthMap, TexCoords + offsets[i] * dir ).r );
	}
    //float depthValue = texture(depthMap, TexCoords).r;
    FragColor = minz; // orthographic
	//FragColor = vec4(vec3(depthValue), 0.0);
	
}