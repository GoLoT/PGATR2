#version 330 core

layout(triangles) in;
layout(line_strip, max_vertices = 8) out;

in vec3 norm[];
out vec3 color;

void main()
{
    gl_Position = gl_in[0].gl_Position;
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(norm[0]*0.3, 0.0);
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
    EndPrimitive();
	gl_Position = gl_in[1].gl_Position;
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
    gl_Position = gl_in[1].gl_Position + vec4(norm[1]*0.3, 0.0);
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
    EndPrimitive();
	gl_Position = gl_in[2].gl_Position;
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
    gl_Position = gl_in[2].gl_Position + vec4(norm[2]*0.3, 0.0);
	color = vec3(1.0,1.0,0.0);
    EmitVertex();
    EndPrimitive();

	// Normal de la cara
	float bari = 1.0/3.0;
	vec4 middlePoint = gl_in[0].gl_Position * bari + gl_in[1].gl_Position * bari + gl_in[2].gl_Position * bari;
	vec3 normal = norm[0] * bari + norm[1] * bari + norm[2] * bari;

	gl_Position = middlePoint;
	color = vec3(1.0,0.0,0.0);
	EmitVertex();
	gl_Position = middlePoint + vec4(normal*0.5,0.0);
	color = vec3(1.0,0.0,0.0);
	EmitVertex();
	EndPrimitive();

}