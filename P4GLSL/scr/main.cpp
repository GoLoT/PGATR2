#include "BOX.h"
#include "auxiliar.h"
#include "PLANE.h"

#include <windows.h>

#include <gl/glew.h>
#include <gl/gl.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cstdlib>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>

#define RAND_SEED 31415926
#define SCREEN_SIZE 500,500

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model = glm::mat4(1.0f);


//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////
float angle = 0.0f;

//VAO
unsigned int vao, normalsVao, pointVao;
int numTris;

//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

unsigned int fbo;
unsigned int colorBuffTexId;
unsigned int depthBuffTexId;
unsigned int uVertexTexPP;
unsigned int vertexBuffTexId;
unsigned int normalTexId;



unsigned int colorTexId;
unsigned int specTexId;

//VAO y VBO del plano
unsigned int planeVAO;
unsigned int planeVertexVBO;
unsigned int triangleVAO, quadVAO;

//Por definir
unsigned int vshader;
unsigned int fshader;
unsigned int program;

//Variables Uniform 
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;

//Texturas Uniform
int uColorTex;
int uSpecTex;

//Atributos
int inPos;
int inColor;
int inNormal;
int inTexCoord;

//////////////////////////////
//Shader de post-proceso
unsigned int postProccesVShader;
unsigned int postProccesFShader;
unsigned int postProccesProgram;
//Uniform
unsigned int uColorTexPP;
//Atributos
int inPosPP;

//////////////////////////////
//Shader de iluminación deferred
unsigned int lightVShader;
unsigned int lightFShader;
unsigned int lightProgram;
int uPosTexLight, uNormalTexLight, uAlbedoTexLight, inPosLight, uViewLight;

//Shader de dibujado de normales
unsigned int normalsProgram;
int inPosNormals, inNormalNormals, uModelViewMatNormals, uModelViewProjMatNormals, uNormalMatNormals;
unsigned int wireframeProgram;
int inPosWireframe, inNormalWireframe, uModelViewMatWireframe, uModelViewProjMatWireframe, uNormalMatWireframe;
unsigned int quadgeoProgram, trisgeoProgram, quadtessProgram, tristessProgram;
int inPosQuadgeo, inPosTrisgeo, inPosQuadtess, inPosTristess;

//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);
void specialFunc(int key, int x, int y);

void renderCube();
void renderCubeNormals();
void renderTesselationOverlay();

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShaderFw(const char *vname, const char *fname);
void initShaderPP(const char *vname, const char *fname);
void initShaderLight(const char *vname, const char *fname);
void initShaderNormals();
void initShaderWireframe();
void initShaderQuadGeo();
void initShaderTrisGeo();
void initShaderQuadTess();
void initShaderTrisTess();
void initObj();
void initPlane();
void initPoint();
void initTriangle();
void initQuad();
void destroy();
void initFBO();
void resizeFBO(unsigned int w, unsigned int h);

bool shadedWireframe = false;


//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName);

//////////////////////////////////////////////////////////////
// Nuevas variables auxiliares
//////////////////////////////////////////////////////////////
enum deferredPreviewMode {
  COLOR,
  NORMALS,
  POSITION,
  DEPTH,
  NOPREVIEW
};

enum geometryShaderMode {
  DRAW_NORMALS,
  DRAW_WIREFRAME,
  DRAW_SHADED
};

enum tesselationOverlayMode {
  OVERLAY_NONE,
  OVERLAY_TRIANGLE,
  OVERLAY_QUAD,
  OVERLAY_TESS_TRIANGLE,
  OVERLAY_TESS_QUAD
};

deferredPreviewMode previewMode = NOPREVIEW;
geometryShaderMode geometryMode = DRAW_SHADED;
tesselationOverlayMode tesselationOverlay = OVERLAY_NONE;

//////////////////////////////////////////////////////////////
// Nuevas funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();
	initShaderFw("../shaders_P4/fwRendering.v2.vert", "../shaders_P4/fwRendering.v2.frag");
	initShaderPP("../shaders_P4/postProcessing.v2.vert",
		"../shaders_P4/postProcessing.v2.frag");
  initShaderLight("../shaders_P4/postProcessing.v2.vert", "../shaders_P4/postProcessing.light.frag");
	initObj();
	initPlane();
	initFBO();
	resizeFBO(SCREEN_SIZE);

  initShaderNormals();
  initShaderWireframe();
  initShaderQuadGeo();
  initShaderTrisGeo();
  initShaderQuadTess();
  initShaderTrisTess();

  initPoint();
  initTriangle();
  initQuad();
	
	glutMainLoop();

	destroy();

	return 0;
}

//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	//glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(SCREEN_SIZE);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas GLSL");

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
  glutSpecialFunc(specialFunc);
}

void initOGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	proj = glm::perspective(glm::radians(60.0f), 1.0f, 1.0f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -2.0f;
}


void destroy()
{
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);

	glDetachShader(postProccesProgram, postProccesVShader);
	glDetachShader(postProccesProgram, postProccesFShader);
	glDeleteShader(postProccesVShader);
	glDeleteShader(postProccesFShader);
	glDeleteProgram(postProccesProgram);

  glDetachShader(lightProgram, lightVShader);
  glDetachShader(lightProgram, lightFShader);
  glDeleteShader(lightVShader);
  glDeleteShader(lightFShader);
  glDeleteProgram(lightProgram);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	if (inPos != -1) glDeleteBuffers(1, &posVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1) glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1) glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);

	glDeleteBuffers(1, &planeVertexVBO);
	glDeleteVertexArrays(1, &planeVAO);


	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &specTexId);

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &colorBuffTexId);
	glDeleteTextures(1, &depthBuffTexId);
	glDeleteTextures(1, &vertexBuffTexId);

}

void initShaderFw(const char *vname, const char *fname)
{
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);
  auto tcsShader = loadShader("../shaders_P4/tessmodel.tcs", GL_TESS_CONTROL_SHADER);
  auto tesShader = loadShader("../shaders_P4/tessmodel.tes", GL_TESS_EVALUATION_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
  glAttachShader(program, tcsShader);
  glAttachShader(program, tesShader);

	glBindAttribLocation(program, 0, "inPos");
	glBindAttribLocation(program, 1, "inColor");
	glBindAttribLocation(program, 2, "inNormal");
	glBindAttribLocation(program, 3, "inTexCoord");


	glLinkProgram(program);

	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);

		char *logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete logString;

		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}

	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

	uColorTex = glGetUniformLocation(program, "colorTex");
	uSpecTex = glGetUniformLocation(program, "specTex");

	inPos = glGetAttribLocation(program, "inPos");
	inColor = glGetAttribLocation(program, "inColor");
	inNormal = glGetAttribLocation(program, "inNormal");
	inTexCoord = glGetAttribLocation(program, "inTexCoord");
}

void initShaderPP(const char *vname, const char *fname)
{

	postProccesVShader = loadShader(vname, GL_VERTEX_SHADER);
	postProccesFShader = loadShader(fname, GL_FRAGMENT_SHADER);

	postProccesProgram = glCreateProgram();
	glAttachShader(postProccesProgram, postProccesVShader);
	glAttachShader(postProccesProgram, postProccesFShader);
	glBindAttribLocation(postProccesProgram, 0, "inPos");
	glLinkProgram(postProccesProgram);
	int linked;
	glGetProgramiv(postProccesProgram, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(postProccesProgram, GL_INFO_LOG_LENGTH, &logLen);
		char *logString = new char[logLen];
		glGetProgramInfoLog(postProccesProgram, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete logString;
		glDeleteProgram(postProccesProgram);
		postProccesProgram = 0;
		exit(-1);
	}
	uColorTexPP = glGetUniformLocation(postProccesProgram, "colorTex");
	inPosPP = glGetAttribLocation(postProccesProgram, "inPos");
	uVertexTexPP = glGetUniformLocation(postProccesProgram, "vertexTex");
}

void initShaderLight(const char *vname, const char *fname)
{

  lightVShader = loadShader(vname, GL_VERTEX_SHADER);
  lightFShader = loadShader(fname, GL_FRAGMENT_SHADER);

  lightProgram = glCreateProgram();
  glAttachShader(lightProgram, lightVShader);
  glAttachShader(lightProgram, lightFShader);
  glBindAttribLocation(lightProgram, 0, "inPos");
  glLinkProgram(lightProgram);
  int linked;
  glGetProgramiv(lightProgram, GL_LINK_STATUS, &linked);
  if (!linked)
  {
    //Calculamos una cadena de error
    GLint logLen;
    glGetProgramiv(lightProgram, GL_INFO_LOG_LENGTH, &logLen);
    char *logString = new char[logLen];
    glGetProgramInfoLog(lightProgram, logLen, NULL, logString);
    std::cout << "Error: " << logString << std::endl;
    delete logString;
    glDeleteProgram(lightProgram);
    lightProgram = 0;
    exit(-1);
  }

  uPosTexLight = glGetUniformLocation(lightProgram, "posTex");
  uNormalTexLight = glGetUniformLocation(lightProgram, "normalTex");
  uAlbedoTexLight = glGetUniformLocation(lightProgram, "albedoSpecTex");
  uViewLight = glGetUniformLocation(lightProgram, "view");
  inPosLight = glGetAttribLocation(lightProgram, "inPos");
}

void initShaderNormals()
{
  auto vertexShader = loadShader("../shaders_P4/drawNormals.vert", GL_VERTEX_SHADER);
  auto fragmentShader = loadShader("../shaders_P4/drawNormals.frag", GL_FRAGMENT_SHADER);
  auto geometryShader = loadShader("../shaders_P4/drawNormals.geo", GL_GEOMETRY_SHADER);

  normalsProgram = glCreateProgram();
  glAttachShader(normalsProgram, vertexShader);
  glAttachShader(normalsProgram, geometryShader);
  glAttachShader(normalsProgram, fragmentShader);
  glBindAttribLocation(normalsProgram, 0, "inPos");
  glBindAttribLocation(normalsProgram, 2, "inNormal");
  glLinkProgram(normalsProgram);
  int linked;
  glGetProgramiv(normalsProgram, GL_LINK_STATUS, &linked);
  if (!linked)
  {
    //Calculamos una cadena de error
    GLint logLen;
    glGetProgramiv(normalsProgram, GL_INFO_LOG_LENGTH, &logLen);
    char *logString = new char[logLen];
    glGetProgramInfoLog(normalsProgram, logLen, NULL, logString);
    std::cout << "Error: " << logString << std::endl;
    delete logString;
    glDeleteProgram(normalsProgram);
    normalsProgram = 0;
    exit(-1);
  }

  inPosNormals = glGetAttribLocation(normalsProgram, "inPos");
  inNormalNormals = glGetAttribLocation(normalsProgram, "inNormal");

  uNormalMatNormals = glGetUniformLocation(normalsProgram, "normal");
  uModelViewMatNormals = glGetUniformLocation(normalsProgram, "modelView");
  uModelViewProjMatNormals = glGetUniformLocation(normalsProgram, "modelViewProj");
}

void initShaderWireframe()
{
  auto vertexShader = loadShader("../shaders_P4/drawNormals.vert", GL_VERTEX_SHADER);
  auto fragmentShader = loadShader("../shaders_P4/drawNormals.frag", GL_FRAGMENT_SHADER);
  auto geometryShader = loadShader("../shaders_P4/drawWireFrame.geo", GL_GEOMETRY_SHADER);

  wireframeProgram = glCreateProgram();
  glAttachShader(wireframeProgram, vertexShader);
  glAttachShader(wireframeProgram, geometryShader);
  glAttachShader(wireframeProgram, fragmentShader);
  glBindAttribLocation(wireframeProgram, 0, "inPos");
  glBindAttribLocation(wireframeProgram, 2, "inNormal");
  glLinkProgram(wireframeProgram);
  int linked;
  glGetProgramiv(wireframeProgram, GL_LINK_STATUS, &linked);
  if (!linked)
  {
    //Calculamos una cadena de error
    GLint logLen;
    glGetProgramiv(wireframeProgram, GL_INFO_LOG_LENGTH, &logLen);
    char *logString = new char[logLen];
    glGetProgramInfoLog(wireframeProgram, logLen, NULL, logString);
    std::cout << "Error: " << logString << std::endl;
    delete logString;
    glDeleteProgram(wireframeProgram);
    normalsProgram = 0;
    exit(-1);
  }

  inPosWireframe = glGetAttribLocation(wireframeProgram, "inPos");
  inNormalWireframe = glGetAttribLocation(wireframeProgram, "inNormal");

  uNormalMatWireframe = glGetUniformLocation(wireframeProgram, "normal");
  uModelViewMatWireframe = glGetUniformLocation(wireframeProgram, "modelView");
  uModelViewProjMatWireframe = glGetUniformLocation(wireframeProgram, "modelViewProj");
}

void initShaderQuadGeo()
{
  auto vertexShader = loadShader("../shaders_P4/geo.vert", GL_VERTEX_SHADER);
  auto fragmentShader = loadShader("../shaders_P4/geo.frag", GL_FRAGMENT_SHADER);
  auto geometryShader = loadShader("../shaders_P4/geoquads.geo", GL_GEOMETRY_SHADER);

  quadgeoProgram = glCreateProgram();
  glAttachShader(quadgeoProgram, vertexShader);
  glAttachShader(quadgeoProgram, geometryShader);
  glAttachShader(quadgeoProgram, fragmentShader);
  glBindAttribLocation(quadgeoProgram, 0, "inPos");
  glLinkProgram(quadgeoProgram);
  int linked;
  glGetProgramiv(quadgeoProgram, GL_LINK_STATUS, &linked);
  if (!linked)
  {
    //Calculamos una cadena de error
    GLint logLen;
    glGetProgramiv(quadgeoProgram, GL_INFO_LOG_LENGTH, &logLen);
    char *logString = new char[logLen];
    glGetProgramInfoLog(quadgeoProgram, logLen, NULL, logString);
    std::cout << "Error: " << logString << std::endl;
    delete logString;
    glDeleteProgram(quadgeoProgram);
    quadgeoProgram = 0;
    exit(-1);
  }

  inPosQuadgeo = glGetAttribLocation(quadgeoProgram, "inPos");
}

void initShaderTrisGeo()
{
  auto vertexShader = loadShader("../shaders_P4/geo.vert", GL_VERTEX_SHADER);
  auto fragmentShader = loadShader("../shaders_P4/geo.frag", GL_FRAGMENT_SHADER);
  auto geometryShader = loadShader("../shaders_P4/geotris.geo", GL_GEOMETRY_SHADER);

  trisgeoProgram = glCreateProgram();
  glAttachShader(trisgeoProgram, vertexShader);
  glAttachShader(trisgeoProgram, geometryShader);
  glAttachShader(trisgeoProgram, fragmentShader);
  glBindAttribLocation(trisgeoProgram, 0, "inPos");
  glLinkProgram(trisgeoProgram);
  int linked;
  glGetProgramiv(trisgeoProgram, GL_LINK_STATUS, &linked);
  if (!linked)
  {
    //Calculamos una cadena de error
    GLint logLen;
    glGetProgramiv(trisgeoProgram, GL_INFO_LOG_LENGTH, &logLen);
    char *logString = new char[logLen];
    glGetProgramInfoLog(trisgeoProgram, logLen, NULL, logString);
    std::cout << "Error: " << logString << std::endl;
    delete logString;
    glDeleteProgram(trisgeoProgram);
    trisgeoProgram = 0;
    exit(-1);
  }

  inPosTrisgeo = glGetAttribLocation(trisgeoProgram, "inPos");
}

void initShaderQuadTess()
{
  auto vertexShader = loadShader("../shaders_P4/geo.vert", GL_VERTEX_SHADER);
  auto tcsShader = loadShader("../shaders_P4/tessquads.tcs", GL_TESS_CONTROL_SHADER);
  auto tesShader = loadShader("../shaders_P4/tessquads.tes", GL_TESS_EVALUATION_SHADER);
  auto fragmentShader = loadShader("../shaders_P4/geo.frag", GL_FRAGMENT_SHADER);

  quadtessProgram = glCreateProgram();
  glAttachShader(quadtessProgram, vertexShader);
  glAttachShader(quadtessProgram, fragmentShader);
  glAttachShader(quadtessProgram, tcsShader);
  glAttachShader(quadtessProgram, tesShader);
  glBindAttribLocation(quadtessProgram, 0, "inPos");
  glLinkProgram(quadtessProgram);
  int linked;
  glGetProgramiv(quadtessProgram, GL_LINK_STATUS, &linked);
  if (!linked)
  {
    //Calculamos una cadena de error
    GLint logLen;
    glGetProgramiv(quadtessProgram, GL_INFO_LOG_LENGTH, &logLen);
    char *logString = new char[logLen];
    glGetProgramInfoLog(quadtessProgram, logLen, NULL, logString);
    std::cout << "Error: " << logString << std::endl;
    delete logString;
    glDeleteProgram(quadtessProgram);
    quadtessProgram = 0;
    exit(-1);
  }

  inPosQuadtess = glGetAttribLocation(quadtessProgram, "inPos");
}

void initShaderTrisTess()
{
  auto vertexShader = loadShader("../shaders_P4/geo.vert", GL_VERTEX_SHADER);
  auto fragmentShader = loadShader("../shaders_P4/geo.frag", GL_FRAGMENT_SHADER);
  auto tcsShader = loadShader("../shaders_P4/tesstris.tcs", GL_TESS_CONTROL_SHADER);
  auto tesShader = loadShader("../shaders_P4/tesstris.tes", GL_TESS_EVALUATION_SHADER);

  tristessProgram = glCreateProgram();
  glAttachShader(tristessProgram, vertexShader);
  glAttachShader(tristessProgram, tcsShader);
  glAttachShader(tristessProgram, tesShader);
  glAttachShader(tristessProgram, fragmentShader);
  glBindAttribLocation(tristessProgram, 0, "inPos");
  glLinkProgram(tristessProgram);
  int linked;
  glGetProgramiv(tristessProgram, GL_LINK_STATUS, &linked);
  if (!linked)
  {
    //Calculamos una cadena de error
    GLint logLen;
    glGetProgramiv(tristessProgram, GL_INFO_LOG_LENGTH, &logLen);
    char *logString = new char[logLen];
    glGetProgramInfoLog(tristessProgram, logLen, NULL, logString);
    std::cout << "Error: " << logString << std::endl;
    delete logString;
    glDeleteProgram(tristessProgram);
    tristessProgram = 0;
    exit(-1);
  }

  inPosTristess = glGetAttribLocation(tristessProgram, "inPos");
}

void initObj()
{

  const struct aiScene* scene = NULL;
  scene = aiImportFile(R"(../model/teapot2.obj)", aiProcess_GenNormals);
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

  numTris = mesh->mNumFaces;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

  glUseProgram(lightProgram);

	if (inPos != -1)
	{
    glGenBuffers(1, &posVBO);
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(float),
      &vertexs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(inPos);
	}

	if (inColor != -1)
	{
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(float),
      &color[0], GL_STATIC_DRAW);
    glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(inColor);
	}

	if (inNormal != -1)
	{
    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float),
      &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(inNormal);
	}

	if (inTexCoord != -1)
	{
    glGenBuffers(1, &texCoordVBO);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
    glBufferData(GL_ARRAY_BUFFER, textCoords.size() * sizeof(float),
      &textCoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(inTexCoord);
	}

  glGenBuffers(1, &triangleIndexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
    mesh->mNumFaces * sizeof(unsigned int) * 3, &faceIdx[0],
    GL_STATIC_DRAW);

  glGenVertexArrays(1, &normalsVao);
  glBindVertexArray(normalsVao);
  glUseProgram(normalsProgram);

  if (inPos != -1)
	{
    GLuint posVBO;
    glGenBuffers(1, &posVBO);
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(float),
      &vertexs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(inPosNormals, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(inPosNormals);
	}

	if (inNormal != -1)
	{
    GLuint normalVBO;
    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float),
      &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(inNormalNormals, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(inNormalNormals);
	}

  GLuint triangleIndexVBO;
  glGenBuffers(1, &triangleIndexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
    mesh->mNumFaces * sizeof(unsigned int) * 3, &faceIdx[0],
    GL_STATIC_DRAW);

	model = glm::mat4(1.0f);

	colorTexId = loadTex("../img/color2.png");
	specTexId = loadTex("../img/emissive.png");
}

void initPoint()
{
  glGenVertexArrays(1, &pointVao);
  glBindVertexArray(pointVao);

  float point[3] = { 0.0f, 0.0f, 0.0f };

  glUseProgram(tristessProgram);
  if (inPos != -1)
  {
    GLuint posVBO;
    glGenBuffers(1, &posVBO);
    glBindBuffer(GL_ARRAY_BUFFER, posVBO);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float),
      point, GL_STATIC_DRAW);
    glVertexAttribPointer(inPosTristess, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(inPosTristess);
  }
}

void initPlane()
{
	glGenVertexArrays(1, &planeVAO);
	glBindVertexArray(planeVAO);
	glGenBuffers(1, &planeVertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVertexVBO);
	glBufferData(GL_ARRAY_BUFFER, planeNVertex * sizeof(float) * 3,
		planeVertexPos, GL_STATIC_DRAW);
	glVertexAttribPointer(inPosPP, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(inPosPP);
}

void initTriangle()
{
  glGenVertexArrays(1, &triangleVAO);
  glBindVertexArray(triangleVAO);

  float vertices[] = { -0.7, -0.7, 0.0,
                       0.7, -0.7, 0.0, 
                       0.0, 0.7, 0.0 };

  GLuint triangleVertexVBO;
  glGenBuffers(1, &triangleVertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, triangleVertexVBO);
  glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * 3,
    vertices, GL_STATIC_DRAW);

  glUseProgram(tristessProgram);
  glVertexAttribPointer(inPosTristess, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(inPosTristess);
}

void initQuad()
{
  glGenVertexArrays(1, &quadVAO);
  glBindVertexArray(quadVAO);

  float vertices[] = { -0.7, 0.7, 0.0,
                       -0.7, -0.7, 0.0,
                       0.7, -0.7, 0.0,
                       0.7, 0.7, 0.0 };

  GLuint quadVertexVBO;
  glGenBuffers(1, &quadVertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVertexVBO);
  glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 3,
    vertices, GL_STATIC_DRAW);

  glUseProgram(quadtessProgram);
  glVertexAttribPointer(inPosQuadtess, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(inPosQuadtess);
}

GLuint loadShader(const char *fileName, GLenum type)
{
	unsigned int fileLen;
	char *source = loadStringFromFile(fileName, fileLen);

	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1,
		(const GLchar **)&source, (const GLint *)&fileLen);
	glCompileShader(shader);
	delete source;

	//Comprobamos que se compilo bien
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
		delete logString;

		glDeleteShader(shader);
		exit(-1);
	}

	return shader;
}

unsigned int loadTex(const char *fileName)
{
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

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	return texId;
}

void renderFunc()
{

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/**/
	glUseProgram(program);

	//Texturas
	if (uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0);
	}

	if (uSpecTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, specTexId);
		glUniform1i(uSpecTex, 1);
	}

	model = glm::mat4(2.0f);
	model[3].w = 1.0f;
	model = glm::rotate(model, angle, glm::vec3(1.0f, 1.0f, 0.0f));
  if (shadedWireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	renderCube();
  if (shadedWireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  if(geometryMode != DRAW_SHADED)
    renderCubeNormals();

  //Omitir etapas posteriores si estamos visualizando etapas intermedias
  if (previewMode == NOPREVIEW) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(lightProgram);

    if (uViewLight != -1)
      glUniformMatrix4fv(uViewLight, 1, GL_FALSE, &(view[0][0]));

    if (uPosTexLight != -1)
    {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, vertexBuffTexId);
      glUniform1i(uPosTexLight, 0);
    }

    if (uNormalTexLight != -1)
    {
      glActiveTexture(GL_TEXTURE0+1);
      glBindTexture(GL_TEXTURE_2D, normalTexId);
      glUniform1i(uNormalTexLight, 1);
    }

    if (uAlbedoTexLight != -1)
    {
      glActiveTexture(GL_TEXTURE0+2);
      glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
      glUniform1i(uAlbedoTexLight, 2);
    }

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (tesselationOverlay != OVERLAY_NONE)
      renderTesselationOverlay();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
  }
  else {

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(postProccesProgram);

    if (uColorTexPP != -1)
    {
      glActiveTexture(GL_TEXTURE0);
      switch (previewMode)
      {
      case NORMALS:
        glBindTexture(GL_TEXTURE_2D, normalTexId);
        break;
      case POSITION:
        glBindTexture(GL_TEXTURE_2D, vertexBuffTexId);
        break;
      case COLOR:
        glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
        break;
      case DEPTH:
        glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
        break;
      default:
        glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
      }
      glUniform1i(uColorTexPP, 0);
    }

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
  }

	glutSwapBuffers();
}

void renderCube()
{
  glUseProgram(program);

	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR)
  {
    std::cout << "Error A " << err << std::endl;
  }

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
		&(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
		&(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
		&(normal[0][0]));

  while ((err = glGetError()) != GL_NO_ERROR)
  {
    std::cout << "Error B " << err << std::endl;
  }

  glPatchParameteri(GL_PATCH_VERTICES, 3);
	glBindVertexArray(vao);
	/*glDrawElements(GL_TRIANGLES, numTris * 3,
		GL_UNSIGNED_INT, (void*)0);*/
  glDrawElements(GL_PATCHES, numTris * 3,
    GL_UNSIGNED_INT, (void*)0);

  while ((err = glGetError()) != GL_NO_ERROR)
  {
    std::cout << "Error C " << err << std::endl;
  }
}

void renderCubeNormals()
{
  switch(geometryMode)
  {
    case DRAW_NORMALS:
      glUseProgram(normalsProgram);
      break;
    case DRAW_WIREFRAME:
      glUseProgram(wireframeProgram);
      break;
  }

  glm::mat4 modelView = view * model;
  glm::mat4 modelViewProj = proj * view * model;
  glm::mat4 normal = glm::transpose(glm::inverse(modelView));
  glDisable(GL_CULL_FACE);


  if (uModelViewMatNormals != -1)
    glUniformMatrix4fv(uModelViewMatNormals, 1, GL_FALSE,
      &(modelView[0][0]));
  if (uModelViewProjMatNormals != -1)
    glUniformMatrix4fv(uModelViewProjMatNormals, 1, GL_FALSE,
      &(modelViewProj[0][0]));
  if (uNormalMatNormals != -1)
    glUniformMatrix4fv(uNormalMatNormals, 1, GL_FALSE,
      &(normal[0][0]));

  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, numTris * 3,
    GL_UNSIGNED_INT, (void*)0);

  glEnable(GL_CULL_FACE);
}

void renderTesselationOverlay()
{
  switch(tesselationOverlay)
  {
  case OVERLAY_QUAD:
    glUseProgram(quadgeoProgram);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(pointVao);
    glDrawArrays(GL_POINTS, 0, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  case OVERLAY_TRIANGLE:
    glUseProgram(trisgeoProgram);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(pointVao);
    glDrawArrays(GL_POINTS, 0, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  case OVERLAY_TESS_QUAD:
    glUseProgram(quadtessProgram);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_PATCHES, 0, 4);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  case OVERLAY_TESS_TRIANGLE:
    glUseProgram(tristessProgram);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glBindVertexArray(triangleVAO);
    glDrawArrays(GL_PATCHES, 0, 3);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  }
}


void resizeFunc(int width, int height)
{
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(60.0f), float(width) /float(height), 1.0f, 550.0f);

	resizeFBO(width, height);

	glutPostRedisplay();
}

void idleFunc()
{
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.02f;
	
	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y){}
void mouseFunc(int button, int state, int x, int y){}

void initFBO() {
	glGenFramebuffers(1, &fbo);
	glGenTextures(1, &colorBuffTexId);
	glGenTextures(1, &depthBuffTexId);
	glGenTextures(1, &vertexBuffTexId);
  glGenTextures(1, &normalTexId);
}

void resizeFBO(unsigned int w, unsigned int h) {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);


	glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0,
		GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, vertexBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0,
		GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, normalTexId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0,
    GL_RGB, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, vertexBuffTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
		GL_TEXTURE_2D, normalTexId, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
    GL_TEXTURE_2D, colorBuffTexId, 0);
	const GLenum buffs[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(3, buffs);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		depthBuffTexId, 0);
	//const GLenum buffs[1] = { GL_COLOR_ATTACHMENT0 };
	//glDrawBuffers(1, buffs);
	if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
		std::cerr << "Error configurando el FBO" << std::endl;
		exit(-1);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

void specialFunc(int key, int x, int y) {
  switch (key) {
  case GLUT_KEY_F1:
    previewMode = NOPREVIEW;
    break;
  case GLUT_KEY_F2:
    previewMode = COLOR;
    break;
  case GLUT_KEY_F3:
    previewMode = NORMALS;
    break;
  case GLUT_KEY_F4:
    previewMode = POSITION;
    break;
  case GLUT_KEY_F5:
    previewMode = DEPTH;
    break;
  case GLUT_KEY_F6:
    geometryMode = (geometryShaderMode)((int) geometryMode + 1);
    if (geometryMode > DRAW_SHADED) geometryMode = (geometryShaderMode) 0;
    break;
  case GLUT_KEY_F7:
    tesselationOverlay = (tesselationOverlayMode)((int)tesselationOverlay + 1);
    if (tesselationOverlay > OVERLAY_TESS_QUAD) tesselationOverlay = (tesselationOverlayMode) 0;
    break;
  case GLUT_KEY_F8:
    //TODO: Enable/disable displacement
    break;
  case GLUT_KEY_F9:
    shadedWireframe = !shadedWireframe;
    break;
  }
  
}

