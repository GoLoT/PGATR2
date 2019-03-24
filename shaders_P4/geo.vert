#version 330 core

in vec3 inPos;

void main()
{
	gl_Position = vec4 (inPos,1.0);
}
