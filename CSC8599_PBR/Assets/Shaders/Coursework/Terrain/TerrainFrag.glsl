#version 330 core

uniform sampler2D diffuseSplatmapTex;
uniform sampler2D diffuseGrassTex;
uniform sampler2D diffuseRocksTex;
uniform sampler2D diffuseGroundTex;

in Vertex 
{
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void) 
{
	//fragColour = texture(diffuseSplatmapTex, IN.texCoord);
	float grassAmount = texture(diffuseSplatmapTex, IN.texCoord / 16.0f).r;
	float rocksAmount = texture(diffuseSplatmapTex, IN.texCoord / 16.0f).g;
	float groundAmount = texture(diffuseSplatmapTex, IN.texCoord / 16.0f).b;

	vec4 grassTex = texture(diffuseGrassTex, IN.texCoord) * grassAmount;
	vec4 rocksTex = texture(diffuseRocksTex, IN.texCoord) * rocksAmount;
	vec4 groundTex = texture(diffuseGroundTex, IN.texCoord) * groundAmount;

	vec4 splatmap = texture(diffuseSplatmapTex, IN.texCoord / 16.0f);

	fragColour = grassTex + rocksTex + groundTex;
}