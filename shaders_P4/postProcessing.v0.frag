#version 330 core
//Color de salida
out vec4 outColor;

//Variables Variantes
in vec2 texCoord;

//Textura
uniform sampler2D colorTex;



void main()
{
//Código del Shader
//NOTA: Sería más adecuado usar texelFetch.
//NOTA: No lo hacemos porque simplifica el paso 5
outColor = vec4(textureLod(colorTex, texCoord,0).rgb, 0.6);
//outColor = vec4(texCoord,vec2(1.0));


}