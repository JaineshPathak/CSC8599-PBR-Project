#version 420 core

uniform sampler2D normalTex;

in Vertex 
{
	vec3 position;
	vec2 texCoord;
	vec3 normal;
	vec3 fragWorldPos;
	vec3 tangent;
	vec3 bitangent;	
} IN;

out vec4 fragColour;

void main(void)
{
	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.bitangent), normalize(IN.normal));
	vec3 N = texture(normalTex, IN.texCoord).rgb * 2.0 - 1.0;
	N = normalize(TBN * normalize(N));

	fragColour = vec4(N.xyz, 1.0);
}