#version 420 core

layout(std140, binding = 4) uniform u_SkyboxData
{
	vec4 skyboxData;
};

uniform samplerCube cubeTex;

in Vertex
{
	vec3 viewDir;
} IN;

out vec4 fragColour;

void main(void)
{
	float m_GAMMA = skyboxData.y;
	float m_Exposure = skyboxData.x;

	vec3 cubeColor = texture(cubeTex, IN.viewDir).rgb;
	
	cubeColor = cubeColor / (cubeColor + vec3(1.0));
	cubeColor = pow(cubeColor, vec3(m_GAMMA));
	cubeColor = vec3(1.0) - exp(-cubeColor * m_Exposure);
	cubeColor = pow(cubeColor, vec3(1.0 / m_GAMMA));

	fragColour = vec4(cubeColor, 1.0);
}