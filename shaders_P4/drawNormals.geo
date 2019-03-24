#version 330 core

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in vec3 norm[];

void main()
{
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(norm[0]*0.3, 0.0);
    EmitVertex();
    EndPrimitive();
	gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    gl_Position = gl_in[1].gl_Position + vec4(norm[1]*0.3, 0.0);
    EmitVertex();
    EndPrimitive();
	gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    gl_Position = gl_in[2].gl_Position + vec4(norm[2]*0.3, 0.0);
    EmitVertex();
    EndPrimitive();
}