#include "OpenGL.h"

#include <string>
#include <vector>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
// Includes to create the cube
#include "ISceneNode.h"
#include "ModelManager.h"
#include "DrawableProxy.h"
#include "DrawableCubeSolution.h"
#include "DrawableSphereSolution.h"
#include "DrawableFloorSolution.h"
#include "DrawableImportedModel.h"
#include "DrawableBackground.h"
#include "DrawableShadowMapping.h"
// Material includes
#include "SolidColorMaterialSolution.h"
#include "ShadedMaterial.h"
#include "MaterialProxy.h"
#include "MaterialManager.h"
#include "ShaderConstantMaterial.h"
#include "Color.h"

#include "BlankTexture2D.h"
#include "TextureBinding.h"
#include "TextureBindManager.h"
#include "TextureBindManagerOpenGL.h"
#include "TextureDataImage.h"
#include "SamplerApplicator.h"
#include "SimpleShaderMaterial.h"
#include "TexParam2DNoMipMap.h"
#include "TexParam2DMipMap.h"

// Includes for the camera
#include "ExaminerCameraNode.h"
#include "PerspectiveTransformSolution.h"
#include "LookAtTransformSolution.h"
#include "ShaderConstantModelView.h"
#include <iostream>
#include <fstream>
// Lights
#include "PointLight.h"
#include "DirectionalLight.h"
#include "LightManager.h"
#include "ShaderConstantLights.h"

#include "RenderTargetProxy.h"
#include "RenderTarget.h"
#include "RenderManager.h"
#include "ClearFrameCommand.h"
#include "SwapCommand.h"

// Includes for walking the scene graph
#include "RenderVisitor.h"
#include "PrintSceneVisitor.h"

// Interaction
std::vector<IMouseHandler*> mouseHandlers;
std::vector<IKeyboardHandler*> keyboardHandlers;


using namespace Crawfis::Graphics;
using namespace Crawfis::Math;
using namespace std;


ISceneNode* rootSceneNode;
ISceneNode* sceneNode01;
ISceneNode* sceneNode02;
ISceneNode* sceneNode03;

IVisitor* renderVisitor;

ExaminerCameraNode* examiner;
ExaminerCameraNode* examiner1;
ExaminerCameraNode* examiner2;
ExaminerCameraNode* examiner3;
ExaminerCameraNode* examiner4;

int renderType;
int windowGUID;
int windowWidth;
int windowHeight; 
#pragma region
GLfloat vertices[] = { -1.0f,-1.0f,
1.0f,-1.0f,
-1.0f,1.0f,
1.0f,1.0f };

// A vertex array object for the points
unsigned int vaoBrushPath;
// A vertex buffer object for the vertices
unsigned int vboBrushPath;
// A Shader program
unsigned int gaussianProgramID, brushProgramID;
GLuint FramebufferName = 0;
int width, height;
const int Max_Points = 512;
std::vector<float> brushPath;
unsigned int brushTexture;
bool painting = false;
#pragma endregion Brush Variables.
void CreateGLUTWindow(std::string windowTitle)
{
	windowWidth = 800;
	windowHeight = 600;
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(windowWidth, windowHeight);
	windowGUID = glutCreateWindow(windowTitle.c_str());
}

void InitializeOpenGLExtensions()
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		throw "Error initializing GLEW";
	}

	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
}

void InitializeDevices()
{
	CreateGLUTWindow("OpenGL Demo Framework");
	InitializeOpenGLExtensions();
	glDisable(GL_CULL_FACE);
}

void CreateLights()
{
	
	PointLight* pointLight = new PointLight("light0-pt");
	pointLight->setPosition(Vector3(-2, 5, -1));

	LightManager::Instance()->SetLight(0, pointLight);
	DirectionalLight* dirLight = new DirectionalLight("light1-dir");
	//dirLight->setColor(Colors::IndianRed);
	dirLight->setDirection(Vector3(10, 1, 1));
	LightManager::Instance()->SetLight(1, dirLight);
}


void CreateTexturedMaterial()
{
	ITextureDataObject* texture = new BlankTexture2D(1024, 1024);
	ITextureDataObject* redTexture = new BlankTexture2D(1024, 1024, Color(1, 0, 0, 1), GL_RGB);
	redTexture->setTextureParams(&TexParam2DNoMipMap::Instance);
	ITextureDataObject* imageTexture = new TextureDataImage("../Media/Textures/UVGrid.jpg", GL_RGB);
	imageTexture->setTextureParams(&TexParam2DMipMap::Instance);
	SamplerApplicator* uniformBinding = new SamplerApplicator("texture");
	TextureBinding* binding = TextureBindManager::Instance()->CreateBinding(imageTexture, uniformBinding);
	binding->Enable();
	binding->Disable();

	VertexRoutine* vertexRoutine = new VertexRoutine("..\\Media\\Shaders\\ShadedTextured-vert.glsl");
	FragmentRoutine* fragmentRoutine = new FragmentRoutine("..\\Media\\Shaders\\Textured.frag");

	IShaderProgram* shaderProgram = new ShaderProgramWithMatrices(vertexRoutine, fragmentRoutine);
	SimpleShaderMaterial* texturedMaterial = new SimpleShaderMaterial(shaderProgram);
	texturedMaterial->setShaderConstant(uniformBinding);
	texturedMaterial->AddTexture(binding);

	ShadedMaterial* white = new ShadedMaterial(shaderProgram);
	white->setAmbientReflection(0.0f*Colors::White);
	white->setDiffuseReflection(0.75f*Colors::White);
	white->setSpecularReflection(0.25f*Colors::White);
	white->setShininess(10.0f);

	ShaderConstantMaterial* materialConstant = new ShaderConstantMaterial("frontMaterial");
	materialConstant->setValue(white);
	ShaderConstantLights* lightConstant = new ShaderConstantLights();
	ShaderConstantCollection* shaderConstantList = new ShaderConstantCollection();
	shaderConstantList->AddConstant(materialConstant);
	shaderConstantList->AddConstant(lightConstant);
	shaderProgram->setShaderConstant(shaderConstantList);

	MaterialManager::Instance()->RegisterMaterial("Textured", texturedMaterial);
}

void CreateImportedModelTexureMaterial() {
	ITextureDataObject* texture = new BlankTexture2D(1024, 1024);
	ITextureDataObject* redTexture = new BlankTexture2D(1024, 1024, Color(1, 0, 0, 1), GL_RGB);
	redTexture->setTextureParams(&TexParam2DNoMipMap::Instance);
	ITextureDataObject* imageTexture = new TextureDataImage("../Media/Objects/grid.jpg", GL_RGB);
	imageTexture->setTextureParams(&TexParam2DMipMap::Instance);
	SamplerApplicator* uniformBinding = new SamplerApplicator("texture");
	TextureBinding* binding = TextureBindManager::Instance()->CreateBinding(imageTexture, uniformBinding);
	binding->Enable();
	binding->Disable();

	ITextureDataObject* normal = new BlankTexture2D(1024, 1024);
	ITextureDataObject* redNormal = new BlankTexture2D(1024, 1024, Color(1, 0, 0, 1), GL_RGB);
	redNormal->setTextureParams(&TexParam2DNoMipMap::Instance);
	ITextureDataObject* imageTexture2 = new TextureDataImage("../Media/Objects/foot_normals.png", GL_RGB);
	imageTexture2->setTextureParams(&TexParam2DMipMap::Instance);
	SamplerApplicator* uniformBinding2 = new SamplerApplicator("normalMap");
	TextureBinding* binding2 = TextureBindManager::Instance()->CreateBinding(imageTexture2, uniformBinding2);
	binding2->Enable();
	binding2->Disable();

	VertexRoutine* vertex = new VertexRoutine("..\\Media\\Shaders\\ModelTexture.vert");
	FragmentRoutine* fragment = new FragmentRoutine("..\\Media\\Shaders\\ModelTexture.frag");

	IShaderProgram* shaderProgram = new ShaderProgramWithMatrices(vertex, fragment);
	SimpleShaderMaterial* texturedMaterial = new SimpleShaderMaterial(shaderProgram);
	texturedMaterial->setShaderConstant(uniformBinding);
	texturedMaterial->AddTexture(binding);

	ShadedMaterial* sm = new ShadedMaterial(shaderProgram);
	sm->setAmbientReflection(0.0f*Colors::White);
	sm->setDiffuseReflection(0.7f*Colors::White);
	sm->setSpecularReflection(0.3f*Colors::White);
	sm->setShininess(10.0f);

	ShaderConstantMaterial* materialConstant = new ShaderConstantMaterial("ConstantMaterial");
	materialConstant->setValue(sm);
	ShaderConstantLights* lightConstant = new ShaderConstantLights();
	ShaderConstantCollection* shaderConstantList = new ShaderConstantCollection();
	shaderConstantList->AddConstant(materialConstant);
	shaderConstantList->AddConstant(lightConstant);
	shaderProgram->setShaderConstant(shaderConstantList);

	MaterialManager::Instance()->RegisterMaterial("ModelTexture", texturedMaterial);
}

void CreateImportedSpotTexureMaterial() {
	ITextureDataObject* texture = new BlankTexture2D(1024, 1024);
	ITextureDataObject* redTexture = new BlankTexture2D(1024, 1024, Color(1, 0, 0, 1), GL_RGB);
	redTexture->setTextureParams(&TexParam2DNoMipMap::Instance);
	ITextureDataObject* imageTexture = new TextureDataImage("../Media/Objects/spot_texture.png", GL_RGB);
	imageTexture->setTextureParams(&TexParam2DMipMap::Instance);
	SamplerApplicator* uniformBinding = new SamplerApplicator("spotTexture");
	TextureBinding* binding = TextureBindManager::Instance()->CreateBinding(imageTexture, uniformBinding);
	binding->Enable();
	binding->Disable();

	ITextureDataObject* normal = new BlankTexture2D(1024, 1024);
	ITextureDataObject* redNormal = new BlankTexture2D(1024, 1024, Color(1, 0, 0, 1), GL_RGB);
	redNormal->setTextureParams(&TexParam2DNoMipMap::Instance);
	ITextureDataObject* imageTexture2 = new TextureDataImage("../Media/Objects/foot_normals.png", GL_RGB);
	imageTexture2->setTextureParams(&TexParam2DMipMap::Instance);
	SamplerApplicator* uniformBinding2 = new SamplerApplicator("normalMap");
	TextureBinding* binding2 = TextureBindManager::Instance()->CreateBinding(imageTexture2, uniformBinding2);
	binding2->Enable();
	binding2->Disable();

	VertexRoutine* vertex = new VertexRoutine("..\\Media\\Shaders\\SpotTexture.vert");
	FragmentRoutine* fragment = new FragmentRoutine("..\\Media\\Shaders\\SpotTexture.frag");

	IShaderProgram* shaderProgram = new ShaderProgramWithMatrices(vertex, fragment);
	SimpleShaderMaterial* texturedMaterial = new SimpleShaderMaterial(shaderProgram);
	texturedMaterial->setShaderConstant(uniformBinding);
	texturedMaterial->AddTexture(binding);

	ShadedMaterial* sm = new ShadedMaterial(shaderProgram);
	sm->setAmbientReflection(0.0f*Colors::White);
	sm->setDiffuseReflection(0.7f*Colors::White);
	sm->setSpecularReflection(0.3f*Colors::White);
	sm->setShininess(10.0f);

	ShaderConstantMaterial* materialConstant = new ShaderConstantMaterial("ConstantMaterial");
	materialConstant->setValue(sm);
	ShaderConstantLights* lightConstant = new ShaderConstantLights();
	ShaderConstantCollection* shaderConstantList = new ShaderConstantCollection();
	shaderConstantList->AddConstant(materialConstant);
	shaderConstantList->AddConstant(lightConstant);
	shaderProgram->setShaderConstant(shaderConstantList);

	MaterialManager::Instance()->RegisterMaterial("SpotTexture", texturedMaterial);
}

void GoldFloor() {
	VertexRoutine* vertexRoutine = new VertexRoutine("..\\Media\\Shaders\\VertexLight.glsl");
	FragmentRoutine* fragmentRoutine = new FragmentRoutine("..\\Media\\Shaders\\SolidColorSolution.frag");
	IShaderProgram* shaderProgram = new ShaderProgramWithMatrices(vertexRoutine, fragmentRoutine);

	Color gold(0.8314f, 0.6863f, 0.2169f, 1.0f);
	ShadedMaterial* shinyGold = new ShadedMaterial(shaderProgram);
	shinyGold->setAmbientReflection(0.01f*gold);
	shinyGold->setDiffuseReflection(0.7f*gold);
	shinyGold->setSpecularReflection(0.25f*gold);
	shinyGold->setShininess(100.0f);

	ShaderConstantMaterial* materialConstant = new ShaderConstantMaterial("frontMaterial");
	materialConstant->setValue(shinyGold);
	ShaderConstantLights* lightConstant = new ShaderConstantLights();
	ShaderConstantCollection* shaderConstantList = new ShaderConstantCollection();
	shaderConstantList->AddConstant(materialConstant);
	shaderConstantList->AddConstant(lightConstant);
	shaderProgram->setShaderConstant(shaderConstantList);

	MaterialManager::Instance()->RegisterMaterial("ShinyGold", shinyGold);
}

void CreateShadowMap() {

	VertexRoutine* vertexRoutine = new VertexRoutine("..\\Media\\Shaders\\GeneratingShadowMap.vert");
	FragmentRoutine* fragmentRoutine = new FragmentRoutine("..\\Media\\Shaders\\GeneratingShadowMap.frag");
	IShaderProgram* shaderProgram = new ShaderProgramWithMatrices(vertexRoutine, fragmentRoutine);

	Color gold(0.8314f, 0.6863f, 0.2169f, 1.0f);
	ShadedMaterial* shinyGold = new ShadedMaterial(shaderProgram);
	shinyGold->setAmbientReflection(0.01f*gold);
	shinyGold->setDiffuseReflection(0.7f*gold);
	shinyGold->setSpecularReflection(0.25f*gold);
	shinyGold->setShininess(100.0f);

	ShaderConstantMaterial* materialConstant = new ShaderConstantMaterial("frontMaterial");
	materialConstant->setValue(shinyGold);
	ShaderConstantLights* lightConstant = new ShaderConstantLights();
	ShaderConstantCollection* shaderConstantList = new ShaderConstantCollection();
	shaderConstantList->AddConstant(materialConstant);
	shaderConstantList->AddConstant(lightConstant);
	shaderProgram->setShaderConstant(shaderConstantList);

	MaterialManager::Instance()->RegisterMaterial("ShadowMapShowing", shinyGold);

}

void DiffuseTextureMap() {
	ITextureDataObject* texture = new BlankTexture2D(1024, 1024);
	ITextureDataObject* redTexture = new BlankTexture2D(1024, 1024, Color(1, 0, 0, 1), GL_RGB);
	redTexture->setTextureParams(&TexParam2DNoMipMap::Instance);
	ITextureDataObject* imageTexture = new TextureDataImage("../Media/Objects/grid.jpg", GL_RGB);
	imageTexture->setTextureParams(&TexParam2DMipMap::Instance);
	SamplerApplicator* uniformBinding = new SamplerApplicator("texture");
	TextureBinding* binding = TextureBindManager::Instance()->CreateBinding(imageTexture, uniformBinding);
	binding->Enable();
	binding->Disable();

	VertexRoutine* vertex = new VertexRoutine("..\\Media\\Shaders\\DiffuseTextureMap.vert");
	FragmentRoutine* fragment = new FragmentRoutine("..\\Media\\Shaders\\DiffuseTextureMap.frag");

	IShaderProgram* shaderProgram = new ShaderProgramWithMatrices(vertex, fragment);
	SimpleShaderMaterial* texturedMaterial = new SimpleShaderMaterial(shaderProgram);
	texturedMaterial->setShaderConstant(uniformBinding);
	texturedMaterial->AddTexture(binding);

	ShadedMaterial* sm = new ShadedMaterial(shaderProgram);
	sm->setAmbientReflection(0.0f*Colors::White);
	sm->setDiffuseReflection(0.7f*Colors::White);
	sm->setSpecularReflection(0.3f*Colors::White);
	sm->setShininess(10.0f);

	ShaderConstantMaterial* materialConstant = new ShaderConstantMaterial("ConstantMaterial");
	materialConstant->setValue(sm);
	ShaderConstantLights* lightConstant = new ShaderConstantLights();
	ShaderConstantCollection* shaderConstantList = new ShaderConstantCollection();
	shaderConstantList->AddConstant(materialConstant);
	shaderConstantList->AddConstant(lightConstant);
	shaderProgram->setShaderConstant(shaderConstantList);

	MaterialManager::Instance()->RegisterMaterial("Diffuse", texturedMaterial);
}





void CreateTextureColorShaders()
{
	VertexRoutine* vertexRoutine = new VertexRoutine("..\\Media\\Shaders\\TextureCoordColor.vert");
	FragmentRoutine* fragmentRoutine = new FragmentRoutine("..\\Media\\Shaders\\TextureCoordColor.frag");
	IShaderProgram* shaderProgram = new ShaderProgramWithMatrices(vertexRoutine, fragmentRoutine);
	SimpleShaderMaterial* texturedMaterial = new SimpleShaderMaterial(shaderProgram);

	ShaderConstantLights* lightConstant = new ShaderConstantLights();
	ShaderConstantCollection* shaderConstantList = new ShaderConstantCollection();
	shaderConstantList->AddConstant(lightConstant);
	shaderProgram->setShaderConstant(shaderConstantList);
	MaterialManager::Instance()->RegisterMaterial("TextureCoord", texturedMaterial);
}

void CreateBackFloorTexture()
{
	ITextureDataObject* texture = new BlankTexture2D(1024, 1024);
	ITextureDataObject* redTexture = new BlankTexture2D(1024, 1024, Color(1, 0, 0, 1), GL_RGB);
	redTexture->setTextureParams(&TexParam2DNoMipMap::Instance);
	ITextureDataObject* imageTexture = new TextureDataImage("../Media/Textures/UVGrid.jpg", GL_RGB);
	imageTexture->setTextureParams(&TexParam2DMipMap::Instance);
	SamplerApplicator* uniformBinding = new SamplerApplicator("texture");
	TextureBinding* binding = TextureBindManager::Instance()->CreateBinding(imageTexture, uniformBinding);
	binding->Enable();
	binding->Disable();

	VertexRoutine* vertexRoutine = new VertexRoutine("..\\Media\\Shaders\\ShadedTextured-vert.glsl");
	FragmentRoutine* fragmentRoutine = new FragmentRoutine("..\\Media\\Shaders\\Textured.frag");

	IShaderProgram* shaderProgram = new ShaderProgramWithMatrices(vertexRoutine, fragmentRoutine);
	SimpleShaderMaterial* texturedMaterial = new SimpleShaderMaterial(shaderProgram);
	texturedMaterial->setShaderConstant(uniformBinding);
	texturedMaterial->AddTexture(binding);

	ShadedMaterial* white = new ShadedMaterial(shaderProgram);
	white->setAmbientReflection(0.0f*Colors::White);
	white->setDiffuseReflection(0.75f*Colors::White);
	white->setSpecularReflection(0.25f*Colors::White);
	white->setShininess(10.0f);

	ShaderConstantMaterial* materialConstant = new ShaderConstantMaterial("frontMaterial");
	materialConstant->setValue(white);
	ShaderConstantLights* lightConstant = new ShaderConstantLights();
	ShaderConstantCollection* shaderConstantList = new ShaderConstantCollection();
	shaderConstantList->AddConstant(materialConstant);
	shaderConstantList->AddConstant(lightConstant);
	shaderProgram->setShaderConstant(shaderConstantList);

	MaterialManager::Instance()->RegisterMaterial("Textured01", texturedMaterial);
}

void CreateBackground()
{
	ITextureDataObject* texture = new BlankTexture2D(1024, 1024);
	ITextureDataObject* redTexture = new BlankTexture2D(1024, 1024, Color(1, 0, 0, 1), GL_RGB);
	redTexture->setTextureParams(&TexParam2DNoMipMap::Instance);
	ITextureDataObject* imageTexture = new TextureDataImage("../Media/Textures/UVGrid.jpg", GL_RGB);
	imageTexture->setTextureParams(&TexParam2DMipMap::Instance);
	SamplerApplicator* uniformBinding = new SamplerApplicator("backgroundTexture");
	TextureBinding* binding = TextureBindManager::Instance()->CreateBinding(imageTexture, uniformBinding);
	binding->Enable();
	binding->Disable();

	VertexRoutine* vertexRoutine = new VertexRoutine("..\\Media\\Shaders\\Background.vert");
	FragmentRoutine* fragmentRoutine = new FragmentRoutine("..\\Media\\Shaders\\Background.frag");

	IShaderProgram* shaderProgram = new ShaderProgramWithMatrices(vertexRoutine, fragmentRoutine);
	SimpleShaderMaterial* texturedMaterial = new SimpleShaderMaterial(shaderProgram);
	texturedMaterial->setShaderConstant(uniformBinding);
	texturedMaterial->AddTexture(binding);

	ShadedMaterial* white = new ShadedMaterial(shaderProgram);
	white->setAmbientReflection(0.0f*Colors::White);
	white->setDiffuseReflection(0.75f*Colors::White);
	white->setSpecularReflection(0.25f*Colors::White);
	white->setShininess(10.0f);

	ShaderConstantMaterial* materialConstant = new ShaderConstantMaterial("frontMaterial");
	materialConstant->setValue(white);
	ShaderConstantLights* lightConstant = new ShaderConstantLights();
	ShaderConstantCollection* shaderConstantList = new ShaderConstantCollection();
	shaderConstantList->AddConstant(materialConstant);
	shaderConstantList->AddConstant(lightConstant);
	shaderProgram->setShaderConstant(shaderConstantList);

	MaterialManager::Instance()->RegisterMaterial("TexturedBackground", texturedMaterial);
}

ISceneNode* CreateSceneGraph()
{
	// Create a simple scene
	// Perspective
	// LookAt camera
	// Drawable Cube
	//
	// First, create the models and register them.
	//DrawableSphereSolution* sphere = new DrawableSphereSolution(10000);
	//sphere->setLevel(6);
	//ModelManager::Instance()->RegisterModel("Sphere", sphere);
	//DrawableSphereSolution* smoothSphere = new DrawableSphereSolution(100000);
	//ModelManager::Instance()->RegisterModel("SmoothSphere", smoothSphere);
	//IDrawable* cube = new DrawableCubeSolution();
	//ModelManager::Instance()->RegisterModel("Cube", cube);
	
	DrawableBackground* myBackground = new DrawableBackground();
	ModelManager::Instance()->RegisterModel("myBackground", myBackground);

	DrawableFloor* floor = new DrawableFloor(10,10);
	ModelManager::Instance()->RegisterModel("Floor", floor);



	//DrawableProxy* sphereNode = new DrawableProxy("Sphere", "SmoothSphere");
	//DrawableProxy* sphereNode2 = new DrawableProxy("Sphere", "Sphere");
	//DrawableProxy* cubeNode = new DrawableProxy("Cube", "Cube"); // It is okay if they have the same name.
	DrawableProxy* floorNode = new DrawableProxy("Floor", "Floor"); // It is okay if they have the same name.
																 // Add a material

	DrawableProxy* backgroundNode = new DrawableProxy("myBackground", "myBackground");
	//Imported model
	DrawableImportedModel* model = new DrawableImportedModel("..\\Media\\Objects\\blub_triangulated.obj",false);
	ModelManager::Instance()->RegisterModel("Model", model);
	DrawableProxy* modelNode = new DrawableProxy("Model", "Model");

	//Imported model
	DrawableImportedModel* spot = new DrawableImportedModel("..\\Media\\Objects\\spot_triangulated.obj", false);
	ModelManager::Instance()->RegisterModel("Spot", spot);
	DrawableProxy* spotNode = new DrawableProxy("Spot", "Spot");

	//Generating shadow maps
	DrawableShadowMapping* shadowMapModel = new DrawableShadowMapping();
	ModelManager::Instance()->RegisterModel("ShadowMapping", shadowMapModel);
	DrawableProxy* shadowMapModelNode = new DrawableProxy("ShadowMapping", "ShadowMapping");

	//SolidColorMaterialSolution* scarlet = new SolidColorMaterialSolution(Colors::White);
	//MaterialManager::Instance()->RegisterMaterial("Scarlet", scarlet);

	CreateLights();
	//CreateTexturedMaterial();
	//CreateTextureColorShaders();
	GoldFloor();
	//DiffuseTextureMap();
	CreateBackground();
	CreateShadowMap();
	CreateBackFloorTexture();
	CreateImportedModelTexureMaterial();
	CreateImportedSpotTexureMaterial();






	MaterialProxy* materialNode = new MaterialProxy("Floor Material", "Textured01", floorNode);
	TransformMatrixNodeSolution* floorTransform = new TransformMatrixNodeSolution("CubeSpace", materialNode);
	floorTransform->Scale(60, 1, 60);
	floorTransform->Translate(0, 5, 0);
	//floorTransform2->Rotate(90, Vector3(0.0, 0.0, 1.0));

	MaterialProxy* shadowMapNode = new MaterialProxy("Shadow Material", "ShadowMapShowing", shadowMapModelNode);
	TransformMatrixNodeSolution* shadowMapTransform = new TransformMatrixNodeSolution("CubeSpace", shadowMapNode);


	MaterialProxy*	modelMaterialNodeTextured;
	MaterialProxy*	backgroundMaterial;
	MaterialProxy*	spotdMaterial;

	modelMaterialNodeTextured = new MaterialProxy("Model Material", "ModelTexture", modelNode);


	backgroundMaterial = new MaterialProxy("Background Material", "TexturedBackground", backgroundNode);

	spotdMaterial = new MaterialProxy("Spot Material", "SpotTexture", spotNode);



	TransformMatrixNodeSolution* modelTransformTextured = new TransformMatrixNodeSolution("CubeSpace", modelMaterialNodeTextured);
	//modelTransformTextured->Scale(0.02, 0.02, 0.02);
	modelTransformTextured->Translate(0, 1, 2);

	TransformMatrixNodeSolution* spotModel = new TransformMatrixNodeSolution("CubeSpace", spotdMaterial);



	TransformMatrixNodeSolution* bkGround = new TransformMatrixNodeSolution("CubeSpace", backgroundMaterial);



	GroupNode* group = new GroupNode("Pedestal");

	if (renderType == 1) {
		//group->AddChild(objectTransformWireframed);
		group->AddChild(bkGround);
		group->AddChild(floorTransform);
		//group->AddChild(background);
		group->AddChild(spotModel);
		group->AddChild(modelTransformTextured);
		examiner1 = new ExaminerCameraNode("Examiner1", group);
		examiner1->setWidth(windowWidth*(3.0f / 4.0f) -2.0f);
		examiner1->setHeight(windowHeight - 2.0f);
		return examiner1;
	}
	else if (renderType == 2) {
		//group->AddChild(floorTransform);
		//group->AddChild(objectTransformWireframed);
		group->AddChild(shadowMapTransform);

		examiner2 = new ExaminerCameraNode("Examiner2", group);
		examiner2->setWidth(windowWidth*(1.0f / 4.0f) - 2.0f);
		examiner2->setHeight(windowHeight*(1.0f / 3.0f) - 2.0f);
		return examiner2;
	}
	else if (renderType == 3) {

		//group->AddChild(texutreCoordColor);

		//group->AddChild(objectTransformTexuted);
		examiner3 = new ExaminerCameraNode("Examiner3", group);
		examiner3->setWidth(windowWidth*(1.0f / 4.0f) - 2.0f);
		examiner3->setHeight(windowHeight*(1.0f / 3.0f) - 2.0f);
		return examiner3;
	}
	else if (renderType == 4) {
		//group->AddChild(diffuse);
		examiner4 = new ExaminerCameraNode("Examiner4", group);
		examiner4->setWidth(windowWidth*(1.0f / 4.0f) - 2.0f);
		examiner4->setHeight(windowHeight*(1.0f / 3.0f) - 2.0f);
		return examiner4;
	}

	//return examiner;
}

void DisplayFrame()
{
	
	glEnable(GL_SCISSOR_TEST);
	//main viewport
	glViewport(0, 0, windowWidth*(4.0f / 4.0f), windowHeight);
	glScissor(0, 0, windowWidth*(4.0f / 4.0f), windowHeight);
	rootSceneNode->Accept(renderVisitor);
	//top small viewport
	glViewport(windowWidth*(3.0f / 4.0f), windowHeight*(2.0f / 3.0f), windowWidth*(1.0f / 4.0f), windowHeight*(1.0f / 3.0f));
	glScissor(windowWidth*(3.0f / 4.0f), windowHeight*(2.0f / 3.0f), windowWidth*(1.0f / 4.0f), windowHeight*(1.0f / 3.0f));
	sceneNode01->Accept(renderVisitor);
	//rootSceneNode->Accept(renderVisitor);
}

void ReshapeWindow(int newWidth, int newHeight)
{
	windowWidth = newWidth;
	windowHeight = newHeight;
	examiner1->setWidth(windowWidth);
	examiner1->setHeight(windowHeight);
	examiner2->setWidth(windowWidth);
	examiner2->setHeight(windowHeight);
	examiner3->setWidth(windowWidth);
	examiner3->setHeight(windowHeight);
	examiner4->setWidth(windowWidth);
	examiner4->setHeight(windowHeight);


	glutPostRedisplay();
	//windowWidth = newWidth;
	//windowHeight = newHeight;
	//examiner->setWidth(windowWidth);
	//examiner->setHeight(windowHeight);
	//glViewport(0, 0, windowWidth, windowHeight);
	//glutPostRedisplay();
}

ISceneNode* CreateFrameBuffer(Crawfis::Graphics::ISceneNode * scene)
{
	IRenderTarget* screen = new RenderTarget();
	RenderManager::Instance()->RegisterRenderTarget("Screen", screen);
	screen->setEnableCommand(new ClearFrameCommand(Colors::IndianRed));
	screen->setDisableCommand(new SwapCommand(true));
	RenderTargetProxy* frameBuffer = new RenderTargetProxy("Screen Display", "Screen", scene);
	return frameBuffer;
}
void KeyboardController(unsigned char key, int x, int y)
{
	printf("Key Pressed: %c\n", key);
	std::vector<IKeyboardHandler*>::iterator handlerIterator;
	for (handlerIterator = keyboardHandlers.begin(); handlerIterator != keyboardHandlers.end(); handlerIterator++)
	{
		(*handlerIterator)->KeyPress(key, x, y);
	}
	glutPostRedisplay();
}

void NumPadController(int key, int x, int y)
{
	std::vector<IKeyboardHandler*>::iterator handlerIterator;
	for (handlerIterator = keyboardHandlers.begin(); handlerIterator != keyboardHandlers.end(); handlerIterator++)
	{
		(*handlerIterator)->NumPadPress(key, x, y);
	}
	glutPostRedisplay();
}

void MousePressController(int button, int state, int ix, int iy)
{

	std::vector<IMouseHandler*>::iterator handlerIterator;
	for (handlerIterator = mouseHandlers.begin(); handlerIterator != mouseHandlers.end(); handlerIterator++)
	{
		(*handlerIterator)->MouseEvent(button, state, ix, iy);
	}
	glutPostRedisplay();
}

void MouseMotionController(int ix, int iy)
{

	std::vector<IMouseHandler*>::iterator handlerIterator;
	for (handlerIterator = mouseHandlers.begin(); handlerIterator != mouseHandlers.end(); handlerIterator++)
	{
		(*handlerIterator)->MouseMoved(ix, iy);
	}
	glutPostRedisplay();
}

void IdleCallback()
{
}
void InitializeDevIL()
{
	::ilInit();
	::iluInit();
	::ilutInit();
	//::ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	//::ilEnable(IL_ORIGIN_SET);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	InitializeDevices();
	InitializeDevIL();
	TextureBindManagerOpenGL::Init();
	MatrixStack::Init();
	renderType = 1;
	ISceneNode* scene01 = CreateSceneGraph();
	renderType = 2;
	ISceneNode* scene02 = CreateSceneGraph();
	renderType = 3;
	ISceneNode* scene03 = CreateSceneGraph();
	renderType = 4;
	ISceneNode* scene04 = CreateSceneGraph();


	rootSceneNode = CreateFrameBuffer(scene01);
	sceneNode01 = CreateFrameBuffer(scene02);
	sceneNode02 = CreateFrameBuffer(scene03);
	sceneNode03 = CreateFrameBuffer(scene04);


	renderVisitor = new RenderVisitor();
	PrintSceneVisitor* printScene = new PrintSceneVisitor();

	rootSceneNode->Accept(printScene);
	sceneNode01->Accept(printScene);
	sceneNode02->Accept(printScene);
	sceneNode03->Accept(printScene);

	examiner = examiner1;
	keyboardHandlers.push_back(examiner);
	mouseHandlers.push_back(examiner);


	glutDisplayFunc(DisplayFrame);
	glutReshapeFunc(ReshapeWindow);
	glutKeyboardFunc(KeyboardController);
	glutSpecialFunc(NumPadController);
	glutMouseFunc(MousePressController);
	glutMotionFunc(MouseMotionController);

	glutMainLoop();

	return 0;
}