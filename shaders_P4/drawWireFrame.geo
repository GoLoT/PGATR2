#version 330 core

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in vec3 norm[];
out vec3 color;
void main()
{
    gl_Position = gl_in[0].gl_Position - vec4(norm[0]*0.001,0.0);
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
    gl_Position = gl_in[1].gl_Position - vec4(norm[1]*0.001,0.0);
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
    EndPrimitive();

	gl_Position = gl_in[1].gl_Position - vec4(norm[1]*0.001,0.0);
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
    gl_Position = gl_in[2].gl_Position - vec4(norm[2]*0.001,0.0);
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
    EndPrimitive(); 

	gl_Position = gl_in[0].gl_Position - vec4(norm[0]*0.001,0.0);
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
    gl_Position = gl_in[2].gl_Position - vec4(norm[2]*0.001,0.0);
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
    EndPrimitive();
}