#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

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

	gl_Position = gl_in[1].gl_Position + vec4(texture(specTex, texCoordGeo[1]).g * normGeo[1], 0.0);
	norm = normGeo[1];
	texCoord = texCoordGeo[1];
	EmitVertex();


	gl_Position = gl_in[2].gl_Position + vec4(texture(specTex, texCoordGeo[2]).g * normGeo[2], 0.0);
	norm = normGeo[2];
	texCoord = texCoordGeo[2];
	EmitVertex();


	EndPrimitive();
}