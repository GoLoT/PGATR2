#version 330 core

layout(triangles) in;
layout(points, max_vertices = 3) out;

in vec3 norm[];
out vec3 color;

void main()
{
    gl_Position = gl_in[0].gl_Position;
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
	EndPrimitive();

	gl_Position = gl_in[1].gl_Position;
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
	EndPrimitive();

	gl_Position = gl_in[2].gl_Position;
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
	EndPrimitive();
    

}