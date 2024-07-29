#version 420 core

//FROM: https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

uniform sampler2D srcTexture;
uniform float filterRadius;

in Vertex
{
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void)
{
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.

    vec3 upSample = vec3(0.0);

	float x = filterRadius;
    float y = filterRadius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===

    vec3 a = texture(srcTexture, vec2(IN.texCoord.x - x, IN.texCoord.y + y)).rgb;
    vec3 b = texture(srcTexture, vec2(IN.texCoord.x,     IN.texCoord.y + y)).rgb;
    vec3 c = texture(srcTexture, vec2(IN.texCoord.x + x, IN.texCoord.y + y)).rgb;

    vec3 d = texture(srcTexture, vec2(IN.texCoord.x - x, IN.texCoord.y)).rgb;
    vec3 e = texture(srcTexture, vec2(IN.texCoord.x,     IN.texCoord.y)).rgb;
    vec3 f = texture(srcTexture, vec2(IN.texCoord.x + x, IN.texCoord.y)).rgb;

    vec3 g = texture(srcTexture, vec2(IN.texCoord.x - x, IN.texCoord.y - y)).rgb;
    vec3 h = texture(srcTexture, vec2(IN.texCoord.x,     IN.texCoord.y - y)).rgb;
    vec3 i = texture(srcTexture, vec2(IN.texCoord.x + x, IN.texCoord.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |

    upSample = e * 4.0;
    upSample += (b + d + f + h) * 2.0;
    upSample += (a + c + g + i);
    upSample *= 1.0 / 16.0;

    fragColour = vec4(upSample, 1.0);
}