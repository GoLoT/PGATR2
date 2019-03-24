#version 330 core

layout(points) in;
layout(points, max_vertices = 1) out;

in vec3 normGeo[];
in vec2 texCoordGeo[];
in vec3 posGeo[];

out vec3 norm;
out vec2 texCoord;
out vec3 pos;

uniform sampler2D specTex;
void main()
{
    gl_Position = gl_in[0].gl_Position + vec4(texture(specTex, texCoordGeo[0]).g * normGeo[0], 0.0);
	
	norm = normGeo[0];
	texCoord = texCoordGeo[0];

    EmitVertex();
	EndPrimitive();
}