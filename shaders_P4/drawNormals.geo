#version 150 core

layout(points) in;
layout(line_strip, max_vertices = 2) out;

in vec3 norm[];

void main()
{
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(norm[0], 0.0) * 0.5;
    EmitVertex();
    EndPrimitive();
}