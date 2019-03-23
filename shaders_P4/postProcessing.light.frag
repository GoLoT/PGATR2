#version 330 core
//Color de salida
out vec4 outColor;

//Variables Variantes
in vec2 texCoord;

//Textura
uniform sampler2D posTex;
uniform sampler2D normalTex;
uniform sampler2D albedoSpecTex;

uniform mat4 view;

//Parametros de la niebla
vec3 cfog = vec3(0.0);
float df = 0.03;

//Parametros de luz globales
float c1 = 1;
float c2 = 0;
float c3 = 0.1;

// todos los paramentros estan dados en cooredenadas globales
//Parametros de la luz 1 - Point
vec3 Il1 = vec3(0.6);
vec3 Ia1 = vec3(0.2);
vec3 pl1 = vec3(0,0,2);
float att1 = 0.2;

//Parametros de la luz 2 - Point
vec3 Il2 = vec3(1.0,0.0,0.0);
vec3 Ia2 = vec3(0.05,0.0,0.0);
vec3 pl2 = vec3(5,0,-3);
float att2 = 0.2;

//light parameters 3 - Directional
vec3 dir = vec3(0.0, 1.0, 1.0);
vec3 Il3 = vec3(0.8);
vec3 Ia3 = vec3(0.0);

//light parameters 4 - Focal
vec3 pl4 = vec3(0.0,0.0,1.5);
vec3 D = vec3(0.0,0.0, -1.0);
float b = 60; // 30º
vec3 Il4 = vec3(1.0);
vec3 Ia4 = vec3(0.0);
float att4 = 0.2;

#define MAX_LIGHTS 10
int numLights;
struct Light {
   vec4 position;
   vec3 intensities;
   float constAtt;
   float linearAtt;
   float cuadraticAtt;
   vec3 Ia;
   float coneAngle;
   vec3 coneDir;
};

//Obj
vec3 Ka;
vec3 Kd;
vec3 Ks;
float brightness;
vec3 Ke;
vec3 pos;

vec3 normalVector;

vec3 shade2(Light[MAX_LIGHTS] lights) {
	vec3 c = vec3(0.0);
	c+= Ke;
	
	vec3 V = normalize(-pos);

	for (int i=0; i< numLights; i++ ) {
		Light light = lights[i];
		//Amibent
		vec3 Iamb = Ka * light.Ia;
		vec3 resi = Iamb;

		vec3 L = normalize(light.position.xyz - pos * light.position.w); //point or directional
		vec3 D = normalize(light.coneDir);
		float cosLD = dot(-L,D);

		if(cosLD >= cos(radians(light.coneAngle))) { // No se ilumninan fragmentos innecesarios
			//Diff
			vec3 Idiff = Kd * light.intensities * max(dot(normalVector,L),0.0);
			Idiff = clamp(Idiff,0.0,1.0);

			//Especular
			vec3 R = normalize(-reflect(L,normalVector));
			vec3 Ispec = Ks * light.intensities * pow( max( dot(R,V), 0.0), brightness); 
			Ispec = clamp(Ispec,0.0,1.0);
			resi += (Ispec + Idiff);
		}

		if ( light.position.w == 1.0) {
			float d = length(light.position.xyz - pos);
			resi /= light.constAtt + ( light.linearAtt * d) +( light.cuadraticAtt * pow(d,2));
		}

		c += resi;
	}

	float f = exp(-pow(df*pos.z,2));
	c = f*c + (1-f) * cfog;
	return  c;
}

void main()
{

	Ka = texture (albedoSpecTex, texCoord).rgb;
	Kd = Ka;
	Ks = texture(albedoSpecTex, texCoord).aaa;
	brightness = 200;
	Ke = vec3(0);
	pos = texture (posTex, texCoord).rgb;
	normalVector = texture(normalTex,texCoord).rgb;
	
	pl1 = (view * vec4(pl1,1.0)).xyz;
	pl2 = (view * vec4(pl2,1.0)).xyz;
	pl4 = (view * vec4(pl4,1.0)).xyz;
	D =  (view * vec4(D,0.0)).xyz;
	dir = (view * vec4(dir,0.0)).xyz; 
	Light lights[MAX_LIGHTS];
	numLights = 4;
	Light l1 = Light(vec4(pl1,1.0), Il1,c1,c2,c3,Ia1,180,vec3(1));
	Light l2 = Light(vec4(pl2,1.0), Il2,c1,c2,c3,Ia2,180,vec3(1));
	Light l3 = Light(vec4(dir,0.0), Il3,0,0,0,Ia3,180,vec3(1));
	Light l4 = Light(vec4(pl4,1.0), Il4,c1,0,0,Ia4,b,D);

	lights[0] = l1;
	lights[1] = l2;
	lights[2] = l3;
	lights[3] = l4;

	outColor = vec4(shade2(lights),0.0);
}