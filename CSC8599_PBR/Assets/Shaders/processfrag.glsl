#version 330 core

uniform sampler2D sceneTex;

uniform int isVertical;
uniform float u_time;
uniform float u_sinTime;

in Vertex 
{
	vec2 texCoord;
} IN;

out vec4 fragColor;

const float scaleFactors[7] = float[](0.006, 0.061, 0.242, 0.383, 0.242, 0.061, 0.006);

mat3 sx = mat3( 
    1.0, 2.0, 1.0, 
    0.0, 0.0, 0.0, 
   -1.0, -2.0, -1.0 
);

mat3 sy = mat3( 
    1.0, 0.0, -1.0, 
    2.0, 0.0, -2.0, 
    1.0, 0.0, -1.0 
);

float zNear = 0.01;
float zFar = 3.0;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));	
}

void main(void) 
{
	//---------------------------------------------------
	
	//Sobel Operator Edge Detection
	/*vec3 diffuse = texture(sceneTex, IN.texCoord).rgb;
	mat3 I;
    for (int i=0; i<3; i++) 
	{
        for (int j=0; j<3; j++) 
		{
            vec3 sample  = texelFetch(sceneTex, ivec2(gl_FragCoord) + ivec2(i-1,j-1), 0).rgb;
            I[i][j] = length(sample); 
		}
	}

	float gx = dot(sx[0], I[0]) + dot(sx[1], I[1]) + dot(sx[2], I[2]); 
	float gy = dot(sy[0], I[0]) + dot(sy[1], I[1]) + dot(sy[2], I[2]);

	float g = sqrt( (gx * gx) + (gy * gy) );
	g = smoothstep(0.0, 5, g);
	fragColor = vec4(mix(diffuse.rgb, vec3(0.0, 1.0, 0.0), g), 1.0);*/

	//-----------------------------------------------------------------------














	//-----------------------------------------------------------------------

	//Grayscale Color
	/*vec3 diffuseColor = texture(sceneTex, IN.texCoord).rgb;
	float grayed = dot(diffuseColor, vec3(0.299, 0.587, 0.114));
	vec4 diffuseGrayed = vec4(grayed, grayed, grayed, 1.0);

	fragColor = diffuseGrayed;*/

	//--------------------------------------------------------------
















	//-----------------------------------------------------------------------

	//Calc Depth
	float z_NDC = gl_FragCoord.z * 2.0 - 1.0;
	float z_Clip = z_NDC / gl_FragCoord.z;
	float depth = LinearizeDepth(gl_FragCoord.z) / zFar;

	//Blur
	fragColor = vec4(0, 0, 0, 1);
	vec2 delta = vec2(0, 0);

	delta = isVertical == 1 ? dFdy(IN.texCoord) : dFdx(IN.texCoord);

//	for (int i = 0; i < 7; i++) 
//	{
//		vec2 offset = delta * (i - 3);
//		vec4 tmp = texture(sceneTex, IN.texCoord.xy + offset);		
//		fragColor += tmp * scaleFactors[i];
//	}

	vec4 unblurredColor = texture(sceneTex, IN.texCoord);
	float brightness = dot(unblurredColor.rgb, vec3(0.299, 0.587, 0.114));
	if(brightness > 0.5)
	{
		//fragColor = unblurredColor;
		vec2 tex_offset = 1.0 / textureSize(sceneTex, 0); // gets size of single texel
		vec3 result = texture(sceneTex, IN.texCoord).rgb * scaleFactors[0]; // current fragment's contribution
		if(isVertical == 0)
		{
			for(int i = 0; i < 7; ++i)
			{
				result += texture(sceneTex, IN.texCoord + vec2(tex_offset.x * i, 0.0)).rgb * scaleFactors[i];
				result += texture(sceneTex, IN.texCoord - vec2(tex_offset.x * i, 0.0)).rgb * scaleFactors[i];
			}
		}
		else if(isVertical == 1)
		{
			for(int i = 0; i < 7; ++i)
			{
				result += texture(sceneTex, IN.texCoord + vec2(0.0, tex_offset.y * i)).rgb * scaleFactors[i];
				result += texture(sceneTex, IN.texCoord - vec2(0.0, tex_offset.y * i)).rgb * scaleFactors[i];
			}
		}
		fragColor = vec4(result, 1.0);
	}
	else
	{
		fragColor = unblurredColor;
	}
	//fragColor = unblurredColor;

	//-----------------------------------------------------------------------
}