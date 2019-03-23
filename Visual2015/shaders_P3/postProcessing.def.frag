#version 330 core
//Color de salida
out vec4 outColor;

//Variables Variantes
in vec2 texCoord;

//Textura
uniform sampler2D dumpTex;

void main()
{
	outColor = texture(dumpTex,texCoord,0.0);
}