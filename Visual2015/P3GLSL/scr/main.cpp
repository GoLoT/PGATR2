#include "BOX.h"
#include "auxiliar.h"

#include <windows.h>

#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model = glm::mat4(1.0f);

//Light Params
glm::vec3 Ia;
glm::vec3 Id;
glm::vec3 Is;
glm::vec3 lpos;

//CamPos
int lastX = 0, lastY = 0;
float yaw = 0, pitch = 0;
glm::vec3 cameraPos = { 0,0, -60.0f };
long long lastT = 0;
const double M_PI = atan(1) * 4;
const double M_2PI = 2 * M_PI;
const float cameraSpeed = 10.0f;
const float cameraSens = 2.0f / 100.0f;
bool turning = false;
glm::vec4 lposCameraAux;
glm::vec3 lposCamera;

//Número de vértices
const int planeNVertex = 4; // 4 vértices 

//Posicíon de los vertices
const float planeVertexPos[] = {
  //Cara z = 1
  -1.0f,	-1.0f,	 0.0f,
   1.0f,	-1.0f,	 0.0f,
  -1.0f,	 1.0f,	 0.0f,
   1.0f,	 1.0f,	 0.0f,
};


//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////

//Texturas Uniform


//VAO
//unsigned int vao;

//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;
//Texturas
unsigned int colorTexId;
unsigned int emiTexId;

int uNormalMat;
int uModelViewMat;
int uModelViewProjMat;
int ulpos;
int uIa;
int uId;
int uIs;
int inPos;
int inColor;
int inNormal;
int inTexCoord;
int uColorTex;
int uEmiTex;

int shadingProgram;
unsigned int vao;
int numTris;
int dumpProgram, dumpColor, dumpPos;
unsigned int planeVAO;
unsigned int planeVertexVBO;


//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);
void specialFunc(int key, int x, int y);
void mouseMotionFunc(int x, int y);

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initObj();
void destroy();
void initTextures(const char *texture1,const char* texture2);


//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName);

//PGATR
void addModel(std::string path);
GLuint createProgram(std::string vertex, std::string fragment);
void createMainFBO();
void createDumpProgram();
unsigned int mainFBO, mainFBOColor;


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();

  shadingProgram = createProgram("../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag");
  addModel(R"(../model/teapot.obj)");
  createMainFBO();
  createDumpProgram();


  lpos = glm::vec3(0, 0, 25.0f);

	glutMainLoop();

	destroy();

	return 0;
}
	
//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv){
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas OGL");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}
	const GLubyte *oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
  glutMotionFunc(mouseMotionFunc);
	glutSpecialFunc(specialFunc);

}

void initOGL(){
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -30;

	lpos = glm::vec3(0.0f, 0.0f, -5.0f);
	Ia = glm::vec3(0.2f);
	Id = glm::vec3(1.0f);
	Is = glm::vec3(1.0f);

	cameraPos = glm::vec3(view[3].x, view[3].y, view[3].z);
}

void destroy() {

  glDeleteBuffers(1, &posVBO);
  glDeleteBuffers(1, &colorVBO);
  glDeleteBuffers(1, &normalVBO);
  glDeleteBuffers(1, &texCoordVBO);

  glDeleteBuffers(1, &triangleIndexVBO);
  glDeleteTextures(1, &colorTexId);
  glDeleteTextures(1, &emiTexId);

}

GLuint loadShader(const char *fileName, GLenum type){ 
	unsigned int fileLen;
	char *source = loadStringFromFile(fileName, fileLen);

	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1,
		(const GLchar **)&source, (const GLint *)&fileLen);
	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compiló bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char *logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteShader(shader);
		exit(-1);
	}

	return shader;
}
unsigned int loadTex(const char *fileName){
	unsigned char *map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);
	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, (GLvoid*)map);

	delete[] map;

	glGenerateMipmap(GL_TEXTURE_2D);

	if (glewIsSupported("GL_EXT_texture_filter_anisotropic")) {
		float maxAni;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAni);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAni);
	}
	else {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);


	return texId; 
}

void renderFunc(){
  view = glm::mat4(1.0f);
  view = glm::rotate(view, pitch, glm::vec3(1.0f, 0, 0));
  view = glm::rotate(view, yaw, glm::vec3(0, 1.0f, 0));
  view = glm::translate(view, cameraPos);

  //glBindFramebuffer(GL_FRAMEBUFFER, mainFBO);
  glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  lposCameraAux = (view * glm::vec4(lpos, 1.0f));
  lposCamera = glm::vec3(lposCameraAux[0], lposCameraAux[1], lposCameraAux[2]);

    glUseProgram(shadingProgram);

  if (uColorTex != -1)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexId);
    glUniform1i(uColorTex, 0);
  }
  if (uEmiTex != -1)
  {
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, emiTexId);
    glUniform1i(uEmiTex, 1);
  }

  glm::mat4 modelView = view * model;
  glm::mat4 modelViewProj = proj * view * model;
  glm::mat4 normal = glm::transpose(glm::inverse(modelView));

  if (uModelViewMat != -1)
    glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
      &(modelView[0][0]));
  if (uModelViewProjMat != -1)
    glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
      &(modelViewProj[0][0]));
  if (uNormalMat != -1)
    glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
      &(normal[0][0]));
  if (ulpos != -1)
    glUniform3fv(ulpos, 1, &(lposCamera[0]));
  if (uIa != -1)
    glUniform3fv(uIa, 1, &(Ia[0]));
  if (uId != -1)
    glUniform3fv(uId, 1, &(Id[0]));
  if (uIs != -1)
    glUniform3fv(uIs, 1, &(Is[0]));

  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, numTris * 3,
    GL_UNSIGNED_INT, (void*)0);

  /*glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT);*/

  /*glUseProgram(dumpProgram);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mainFBOColor);
  glUniform1i(dumpColor, 0);*/

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  glBindVertexArray(planeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, planeVertexVBO);
  glVertexAttribPointer(dumpPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR)
  {
    std::cout << "Error OGL " << err << std::endl;
  }
  

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

	glutSwapBuffers();
}

void resizeFunc(int width, int height) {
	glViewport(0, 0, width, height);
  const float n = 1;
  const float f = 100;
  proj = glm::mat4(0);
  proj[1].y = 1.0f / tan(3.14159f / 6.0f);
  proj[2].z = -(n + f) / (f - n);
  proj[3].z = -2.0f*f*n / (f - n);
  proj[2].w = -1;

  proj[0].x = proj[1].y / ((float)width / (float)height);

}

void idleFunc() {
  const long long t = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()
    ).count();

	glutPostRedisplay();
}
void keyboardFunc(unsigned char key, int x, int y){

  const long long t = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()
    ).count();
  double delta = 0;
  delta = (t - lastT) / 1000.0f;
  lastT = t;
  if (delta > 0.016)
    delta = 0.016;

  glm::vec3 forward = glm::vec3(view[0][2], view[1][2], view[2][2]);
  if (forward.z != 0 && forward.x != 0) {
    forward.y = 0;
    forward = glm::normalize(forward);
  }
  glm::vec3 right = glm::vec3(view[0][0], view[1][0], view[2][0]);
  glm::vec3 up = glm::vec3(0, -1.0f, 0);

  switch (key)
  {
  case 'w':
  case 'W':
    forward *= delta * cameraSpeed;
    cameraPos += forward;
    break;
  case 's':
  case 'S':
    forward *= -delta * cameraSpeed;
    cameraPos += forward;
    break;
  case 'a':
  case 'A':
    right *= delta * cameraSpeed;
    cameraPos += right;
    break;
  case 'd':
  case 'D':
    right *= -delta * cameraSpeed;
    cameraPos += right;
    break;
  case 'e':
  case 'E':
    yaw += (float)delta * cameraSpeed / 2;
    if (yaw > M_2PI)
      yaw -= (float)M_2PI;
    break;
  case 'q':
  case 'Q':
    yaw -= (float)delta * cameraSpeed / 2;
    if (yaw < 0)
      yaw += (float)M_2PI;
    break;
  case ' ':
    up *= delta * cameraSpeed;
    cameraPos += up;
    break;
  case 'z':
  case 'Z':
    up *= -delta * cameraSpeed;
    cameraPos += up;
    break;
  case '+':
    Ia += 0.05;
    Id += 0.05;
    Is += 0.05;
    break;
  case '-':
    Ia -= 0.05;
    Id -= 0.05;
    Is -= 0.05;
    break;
  default:
    std::cout << "Se ha pulsado la tecla " << key << std::endl << std::endl;
  }
	glutPostRedisplay();
	
}
void mouseFunc(int button, int state, int x, int y)
{
  if (state == 0)
    std::cout << "Se ha pulsado el botón ";
  else
    std::cout << "Se ha soltado el botón ";

  if (button == 0) std::cout << "de la izquierda del ratón " << std::endl;
  if (button == 1) std::cout << "central del ratón " << std::endl;
  if (button == 2) std::cout << "de la derecha del ratón " << std::endl;

  std::cout << "en la posición " << x << " " << y << std::endl << std::endl;

  if (button == 0) {
    lastX = x;
    lastY = y;
    turning = !state;
  }
}

void mouseMotionFunc(int x, int y)
{
  if (!turning)
    return;
  std::cout << x << std::endl;
  int delta = x - lastX;
  yaw += delta * cameraSens;
  if (yaw > M_2PI)
    yaw -= (float)M_2PI;
  if (yaw < 0)
    yaw += (float)M_2PI;

  delta = y - lastY;
  pitch += delta * cameraSens;
  //Clamp a -170º, 170º
  pitch = glm::clamp(pitch, -2.96706f, 2.96706f);

  lastX = x; lastY = y;
}

void specialFunc(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		lpos[2] -= 0.5f;
		break;
	case GLUT_KEY_DOWN:
		lpos[2] += 0.5f;
		break;
	case GLUT_KEY_RIGHT:
		lpos[0] += 0.5f;
		break;
	case GLUT_KEY_LEFT:
		lpos[0] -= 0.5f;
		break;
  case GLUT_KEY_PAGE_UP:
    lpos[1] += 0.5f;
    break;
  case GLUT_KEY_PAGE_DOWN:
    lpos[1] -= 0.5f;
    break;
		
	}

	glutPostRedisplay();
}

GLuint createProgram(std::string vertex, std::string fragment)
{
  GLuint vshader = loadShader(vertex.c_str(), GL_VERTEX_SHADER);
  GLuint fshader = loadShader(fragment.c_str(), GL_FRAGMENT_SHADER);

  GLuint program = glCreateProgram();
  glAttachShader(program, vshader);
  glAttachShader(program, fshader);

  glLinkProgram(program);

  int linked;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (!linked) {
    //Calculamos una cadena de error
    GLint logLen;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
    char *logString = new char[logLen];
    glGetProgramInfoLog(program, logLen, NULL, logString);
    std::cout << "Error: " << logString << std::endl;
    delete[] logString;
    glDeleteProgram(program);
    program = 0;
    exit(-1);
  }

  glBindAttribLocation(program, 0, "inPos");
  glBindAttribLocation(program, 1, "inColor");
  glBindAttribLocation(program, 2, "inNormal");
  glBindAttribLocation(program, 3, "inTexCoord");


  uNormalMat = glGetUniformLocation(program, "normal");
  uModelViewMat = glGetUniformLocation(program, "modelView");
  uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");
  ulpos = glGetUniformLocation(program, "lpos");
  uIa = glGetUniformLocation(program, "Ia");
  uId = glGetUniformLocation(program, "Id");
  uIs = glGetUniformLocation(program, "Is");

  inPos = glGetAttribLocation(program, "inPos");
  inColor = glGetAttribLocation(program, "inColor");
  inNormal = glGetAttribLocation(program, "inNormal");
  inTexCoord = glGetAttribLocation(program, "inTexCoord");

  uColorTex = glGetUniformLocation(program, "colorTex");
  uEmiTex = glGetUniformLocation(program, "emiTex");

  return program;
}

void addModel(std::string path) {
  const struct aiScene* scene = NULL;
  scene = aiImportFile(path.c_str(), aiProcess_GenNormals);
  auto scene2 = aiApplyPostProcessing(scene, aiProcess_CalcTangentSpace);
  auto mesh = scene2->mMeshes[0];

  std::vector<float> normals;
  normals.reserve(mesh->mNumVertices * 3);
  std::vector<float> vertexs;
  vertexs.reserve(mesh->mNumVertices * 3);
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    vertexs.push_back(mesh->mVertices[i].x);
    vertexs.push_back(mesh->mVertices[i].y);
    vertexs.push_back(mesh->mVertices[i].z);
    normals.push_back(mesh->mNormals[i].x);
    normals.push_back(mesh->mNormals[i].y);
    normals.push_back(mesh->mNormals[i].z);

  }


  std::vector<unsigned int> faceIdx;
  faceIdx.reserve(mesh->mNumFaces * 3);
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    faceIdx.push_back(mesh->mFaces[i].mIndices[0]);
    faceIdx.push_back(mesh->mFaces[i].mIndices[1]);
    faceIdx.push_back(mesh->mFaces[i].mIndices[2]);
  }
  std::vector<float> color(mesh->mNumVertices * 3, 0.5f);

  std::vector<float> textCoords;
  if (mesh->mTextureCoords[0] != nullptr) {
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
      textCoords.push_back(mesh->mTextureCoords[0][i].x);
      textCoords.push_back(mesh->mTextureCoords[0][i].y);
      //textCoords.push_back(mesh->mTextureCoords[0][i].z);
    }
  }
  std::vector<float> tangents;

  if (mesh->HasTangentsAndBitangents()) {
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
      tangents.push_back(mesh->mTangents[i].x);
      tangents.push_back(mesh->mTangents[i].y);
      tangents.push_back(mesh->mTangents[i].z);
    }

  }

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  numTris = mesh->mNumFaces;

  glGenBuffers(1, &posVBO);
  glBindBuffer(GL_ARRAY_BUFFER, posVBO);
  glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(float),
    &vertexs[0], GL_STATIC_DRAW);
  glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(inPos);

  glGenBuffers(1, &colorVBO);
  glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
  glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(float),
    &color[0], GL_STATIC_DRAW);
  glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(inColor);

  glGenBuffers(1, &normalVBO);
  glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
  glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float),
    &normals[0], GL_STATIC_DRAW);
  glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(inNormal);

  glGenBuffers(1, &texCoordVBO);
  glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
  glBufferData(GL_ARRAY_BUFFER, textCoords.size() * sizeof(float),
    &textCoords[0], GL_STATIC_DRAW);
  glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(inTexCoord);

  glGenBuffers(1, &triangleIndexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
    mesh->mNumFaces * sizeof(unsigned int) * 3, &faceIdx[0],
    GL_STATIC_DRAW);

  colorTexId = loadTex("../img/color2.png");
  emiTexId = loadTex("../img/emissive.png");
  model = glm::mat4(1.0f);
}

void createMainFBO()
{
  glGenFramebuffers(1, &mainFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, mainFBO);

  glGenTextures(1, &mainFBOColor);
  glBindTexture(GL_TEXTURE_2D, mainFBOColor);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mainFBOColor, 0);

  unsigned int rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, attachments);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "Error al crear el FBO principal" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void createDumpProgram()
{
  auto vdumpShader = loadShader("../shaders_P3/postProcessing.v2.vert", GL_VERTEX_SHADER);
  auto fdumpShader = loadShader("../shaders_P3/postProcessing.def.frag", GL_FRAGMENT_SHADER);

  dumpProgram = glCreateProgram();
  glAttachShader(dumpProgram, vdumpShader);
  glAttachShader(dumpProgram, fdumpShader);
  glBindAttribLocation(dumpProgram, 0, "inPos");
  glLinkProgram(dumpProgram);
  int linked;
  glGetProgramiv(dumpProgram, GL_LINK_STATUS, &linked);
  if (!linked)
  {
    //Calculamos una cadena de error
    GLint logLen;
    glGetProgramiv(dumpProgram, GL_INFO_LOG_LENGTH, &logLen);
    char *logString = new char[logLen];
    glGetProgramInfoLog(dumpProgram, logLen, NULL, logString);
    std::cout << "Error: " << logString << std::endl;
    delete logString;
    glDeleteProgram(dumpProgram);
    dumpProgram = 0;
    exit(-1);
  }
  dumpColor = glGetUniformLocation(dumpProgram, "dumpTex");
  //glEnableVertexAttribArray(dumpColor);
  dumpPos = glGetAttribLocation(dumpProgram, "inPos");
  glEnableVertexAttribArray(dumpPos);
}

void initPlane()
{
  glGenVertexArrays(1, &planeVAO);
  glBindVertexArray(planeVAO);
  glGenBuffers(1, &planeVertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, planeVertexVBO);
  glBufferData(GL_ARRAY_BUFFER, planeNVertex * sizeof(float) * 3,
    planeVertexPos, GL_STATIC_DRAW);
}