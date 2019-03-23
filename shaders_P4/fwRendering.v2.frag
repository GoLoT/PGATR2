#version 330 core

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec4 outColorSpec;

in vec3 color;
in vec3 pos;
in vec3 norm;
in vec2 texCoord;

uniform sampler2D colorTex;
uniform sampler2D specTex;


void main()
{
	outPos = pos;
	outNormal = normalize(norm);
	outColorSpec.rgb = texture(colorTex, texCoord).rgb;
	outColorSpec.a = texture(specTex, texCoord).r;
}