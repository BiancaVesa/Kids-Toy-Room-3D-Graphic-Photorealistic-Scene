#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightRotation;

// light parameters
glm::vec3 lightDir;
glm::vec3 spotLightDir;
glm::vec3 pointLightPos;
glm::vec3 spotLightPos;
glm::vec3 lightColor;
glm::vec4 pointLightPosV;
glm::vec4 spotLightPosV;

glm::vec3 night;

// shader uniform locations
GLuint alphaLoc;
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint spotLightDirLoc;
GLuint lightColorLoc;
GLuint pointLightPosLoc;
GLuint spotLightPosLoc;
GLuint nightLoc;

// camera
gps::Camera myCamera(
	glm::vec3(0.0f, 0.0f, 3.0f),
	glm::vec3(0.0f, 0.0f, -10.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.05f;
GLfloat alpha;

GLboolean pressedKeys[1024];
GLboolean pressedRightButton;
GLboolean pressedLeftButton;
GLboolean mouseMotionLeft;
GLboolean mouseMotionRight;
GLboolean scrollForward;
GLboolean scrollBackward;

GLfloat xPlane = -1.55f;
GLfloat yPlane = -0.57f;
GLfloat zPlane = -0.72f;

GLfloat xTemp = 0.0f;
GLfloat yTemp = 0.0f;
GLfloat zTemp = 0.0f;

GLfloat xBalloon = 0.0f;
GLfloat yBalloon = 0.0f;
GLfloat zBalloon = -1.42f;
GLfloat scaleFactor = 1;
GLfloat scaleFactorInc = scaleFactor / 1000;


// models
gps::Model3D toyPlane;
gps::Model3D fox;
gps::Model3D cat;
gps::Model3D catToy;
gps::Model3D crayons;
gps::Model3D dice;
gps::Model3D barbieDoll;
gps::Model3D dollHouse;
gps::Model3D legoFigurine;
gps::Model3D numberedDice;
gps::Model3D paperDoll;
gps::Model3D racket;
gps::Model3D sled;
gps::Model3D soccerBall;
gps::Model3D dogToy;
gps::Model3D tennisBall;
gps::Model3D bike;
gps::Model3D room;
gps::Model3D rug;
gps::Model3D mug;
gps::Model3D pony;
gps::Model3D ponyHouse;
gps::Model3D truckToy;
gps::Model3D shelf;
gps::Model3D picture;
gps::Model3D frame;
gps::Model3D books;
gps::Model3D balloon;
gps::Model3D screenQuad;
gps::Model3D movingPlane;


GLfloat angle = 0;
GLfloat anglePlane = 0;
GLfloat moveForward = 0;
GLfloat lightAngle;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

// shaders
gps::Shader myBasicShader;
gps::Shader skyboxShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

gps::SkyBox mySkyBox;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

float lastX = myWindow.getWindowDimensions().width;
float lastY = myWindow.getWindowDimensions().height;
bool down = true;
bool up = false;
bool movePlane = false;
bool stopMoving = false;
bool startOpeningScene = true;


GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
		case GL_INVALID_ENUM:
			error = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error = "INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			error = "INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			error = "STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			error = "STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY:
			error = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);

	glViewport(0, 0, width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			pressedKeys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			pressedKeys[key] = false;
		}
	}

	if (key == GLFW_KEY_C && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

	if (xpos < lastX)
	{
		mouseMotionLeft = true;

		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}
	else if (xpos > lastX)
	{
		mouseMotionRight = true;

		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}
	else if (xpos == lastX)
	{
		mouseMotionLeft = false;
		mouseMotionRight = false;
	}

	lastX = xpos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	std::cout << yoffset << " " << lastY;
	if (yoffset < lastY)
	{
		scrollBackward = true;
	}
	else if (yoffset > lastY)
	{
		scrollForward = true;
	}
	else if (yoffset == lastY)
	{
		scrollBackward = false;
		scrollForward = false;
	}


	lastY = yoffset;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pressedLeftButton = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		pressedLeftButton = false;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		pressedRightButton = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		pressedRightButton = false;
	}
}


void processMovement() {
	if (pressedKeys[GLFW_KEY_W] || scrollForward) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_S] || scrollBackward) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_A] || mouseMotionLeft) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_D] || mouseMotionRight) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_T]) {
		myCamera.move(gps::MOVE_UP, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_G]) {
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_Q] || pressedLeftButton) {
		angle -= 1.0f;
		// update model matrix for teapot
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
		// update normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_E] || pressedRightButton) {
		angle += 1.0f;
		// update model matrix for teapot
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
		// update normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_X]) {

		myBasicShader.useShaderProgram();

		night.x = !night.x;
		night.y = !night.y;
		night.z = !night.z;

		glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "night"), 1, glm::value_ptr(night));
	}

	if (pressedKeys[GLFW_KEY_Z] && stopMoving == false) {
		movePlane = true;
	}

	if (pressedKeys[GLFW_KEY_V]) {
		angle -= 0.5f;
	}

	if (pressedKeys[GLFW_KEY_B]) {
		angle += 0.5f;
	}

	if (pressedKeys[GLFW_KEY_I]) {
		zTemp -= 0.01f;
		zBalloon -= 0.01f;
	}

	if (pressedKeys[GLFW_KEY_Y]) {
		zTemp += 0.01f;
		zBalloon += 0.01f;
	}

	if (pressedKeys[GLFW_KEY_K]) {
		xTemp += 0.01f;
		xBalloon += 0.01f;
	}

	if (pressedKeys[GLFW_KEY_H]) {
		xTemp -= 0.01f;
		xBalloon -= 0.01f;
	}

	if (pressedKeys[GLFW_KEY_U]) {
		yTemp += 0.01f;
	}

	if (pressedKeys[GLFW_KEY_J]) {
		yTemp -= 0.01f;
	}

	if (pressedKeys[GLFW_KEY_N]) {
		scaleFactor += scaleFactorInc;
	}

	if (pressedKeys[GLFW_KEY_M]) {
		scaleFactor -= scaleFactorInc;
	}

	if (pressedKeys[GLFW_KEY_ENTER]) {
		std::cout << "xTemp: " << xTemp << " yTemp: " << yTemp << " zTemp: " << zTemp << " angle: " << angle << " scaleFactor: " << scaleFactor << std::endl;
	}

	if (pressedKeys[GLFW_KEY_L]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);    // wireframe mode
	}

	if (pressedKeys[GLFW_KEY_F]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);    // solid mode
	}

	if (pressedKeys[GLFW_KEY_P]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);   // point mode
	}
}

void initOpenGLWindow() {
	myWindow.Create(1366, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
	glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
	glfwSetMouseButtonCallback(myWindow.getWindow(), mouse_button_callback);
	//glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
	//glfwSetScrollCallback(myWindow.getWindow(), scroll_callback);
	//glfwSetInputMode(myWindow.getWindow(), GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
	//glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {

	screenQuad.LoadModel("quad/quad.obj");
	room.LoadModel("models/room/Room/Sketchfab_2020_02_08_20_59_54.obj");
	movingPlane.LoadModel("models/woodenPlane/Wooden_Plane.obj");
	rug.LoadModel("models/rug/rug.obj");
	numberedDice.LoadModel("models/numberedDice/Dice_Set/Dice_Set/DiceSet.obj");
	bike.LoadModel("models/smallBike/Wooden_bicycle.obj");
	mug.LoadModel("models/mug/Break.obj");
	pony.LoadModel("models/pony/Pony.obj");
	ponyHouse.LoadModel("models/ponyHouse/Sugarcube_Corner.obj");
	fox.LoadModel("models/toyFox/obj/obj/obj.obj");
	sled.LoadModel("models/sled/SledNew_obj/SledNew.obj");
	dollHouse.LoadModel("models/dollHouse/10587_Doll_House_v3_L2.obj");
	racket.LoadModel("models/racket/10540_Tennis_racket_V2_L3.obj");
	tennisBall.LoadModel("models/tennisball/10539_tennis_ball_L3.obj");
	soccerBall.LoadModel("models/soccer/Sketchfab_2020_08_23_19_50_55.obj");
	barbieDoll.LoadModel("models/doll/10578_barbiedoll_v1_L3.obj");
	toyPlane.LoadModel("models/planeToy/ToyPlane_OBJ/ToyPlane/ToyPlane.obj");
	dogToy.LoadModel("models/stuffedToy/11706_stuffed_animal_L2.obj");
	crayons.LoadModel("models/crayons/11676_Crayons_v1_L3.obj");
	catToy.LoadModel("models/catToy/20430_Cat_v1_NEW.obj");
	paperDoll.LoadModel("models/paperDoll/11679_doll_v3_L3.obj");
	legoFigurine.LoadModel("models/legoMiniFigurine/lego.obj");
	truckToy.LoadModel("models/truckToy/Leksaksbil.obj");
	balloon.LoadModel("models/balloon/smeerws_2018-02-16_12-52-58.obj");
	shelf.LoadModel("models/shelf/shelf/shelf.obj");
	picture.LoadModel("models/picture/dog.obj");
	frame.LoadModel("models/largeFrame/frame.obj");
	books.LoadModel("models/book/books.obj");
}

void initShaders() {
	myBasicShader.loadShader(
		"shaders/basic.vert",
		"shaders/basic.frag");

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();

	depthMapShader.loadShader("shaders/lightSpaceShader.vert", "shaders/lightSpaceShader.frag");
	depthMapShader.useShaderProgram();

	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
}


void initUniforms() {
	myBasicShader.useShaderProgram();

	// create model matrix for teapot
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// compute normal matrix for teapot
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
		(float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
		0.1f, 20.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)

	lightDir = glm::vec3(0.0f, 1.0f, 3.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	spotLightDir = glm::vec3(0.0f, -10.0f, 0.0f);
	spotLightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightDir");
	// send light dir to shader
	glUniform3fv(spotLightDirLoc, 1, glm::value_ptr(spotLightDir));

	pointLightPos = glm::vec3(-0.919999f, 0.45f, -0.54f);
	pointLightPosV = glm::vec4(pointLightPos, 1.0f);
	pointLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPosEye");
	glm::vec4 ptr = glm::mat4(view) * pointLightPosV;
	glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(glm::vec3(ptr)));

	spotLightPos = glm::vec3(0.62f, 1.09f, 1.12f);
	spotLightPosV = glm::vec4(spotLightPos, 1.0f);
	spotLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPosEye");
	glm::vec4 ptr1 = glm::mat4(view) * spotLightPosV;
	glUniform3fv(spotLightPosLoc, 1, glm::value_ptr(glm::vec3(ptr1)));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	night = glm::vec3(0.0f, 0.0f, 0.0f);
	nightLoc = glGetUniformLocation(myBasicShader.shaderProgram, "night");
	// send light color to shader
	glUniform3fv(nightLoc, 1, glm::value_ptr(night));

}

void getPointLightPos() {

	myBasicShader.useShaderProgram();

	pointLightPos = glm::vec3(-0.919999f, 0.45f, -0.54f);   // floor lamp

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	pointLightPos = glm::vec3(model * glm::vec4(pointLightPos, 1.0f));

	pointLightPosV = glm::vec4(pointLightPos, 1.0f);

	pointLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPosEye");
	glm::vec4 ptr = glm::mat4(view) * pointLightPosV;
	glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(glm::vec3(ptr)));

}

void getSpotLightPos() {

	myBasicShader.useShaderProgram();

	spotLightPos = glm::vec3(0.62f, 1.09f, 1.12f);    // desk lamp

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	spotLightPos = glm::vec3(model * glm::vec4(spotLightPos, 1.0f));

	spotLightPosV = glm::vec4(spotLightPos, 1.0f);

	spotLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPosEye");
	glm::vec4 ptr = glm::mat4(view) * spotLightPosV;
	glUniform3fv(spotLightPosLoc, 1, glm::value_ptr(glm::vec3(ptr)));

}
void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO

	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix

	glm::mat4 lightView = glm::lookAt(glm::vec3(lightRotation * (glm::vec4(lightDir, 1.0f))), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	const GLfloat near_plane = 0.1f, far_plane = 10.0f;
	glm::mat4 lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, far_plane);

	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}


void renderRoom(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-0.18f, -0.38f, -0.1f));

	model = glm::scale(model, glm::vec3(0.6f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	room.Draw(shader);
}

void renderCrayons(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-92.1f), glm::vec3(1, 0, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(0.77f, -0.74f, 0.0f));

	model = glm::scale(model, glm::vec3(0.0100093f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	crayons.Draw(shader);
}

void renderBike(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(0.25f, -0.565f, -1.18f));

	model = glm::scale(model, glm::vec3(0.370001));


	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	bike.Draw(shader);
}

void renderRug(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-0.04f, -0.555f, 0.0f));

	model = glm::scale(model, glm::vec3(0.0100007f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	rug.Draw(shader);
}

void renderMug(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(63.5f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-0.68f, 0.03f, 1.1f));

	model = glm::scale(model, glm::vec3(0.0400007));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	mug.Draw(shader);
}

void renderFox(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(96.5f), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-1.01f, -0.2f, 0.2f));

	model = glm::scale(model, glm::vec3(0.0100007f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	fox.Draw(shader);
}

void renderSled(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-89.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(0.69f, -0.56f, 1.0f));

	model = glm::scale(model, glm::vec3(0.55f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	sled.Draw(shader);
}

void renderDollHouse(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-92.5f), glm::vec3(1, 0, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-0.51f, -0.26f, -0.59f));

	model = glm::scale(model, glm::vec3(0.00600933f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	dollHouse.Draw(shader);
}

void renderRacket(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-14.5f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(90.5f), glm::vec3(1, 0, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-0.32f, 0.839999f, 0.53f));

	model = glm::scale(model, glm::vec3(0.00700933));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	racket.Draw(shader);
}

void renderTennisBall(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-0.42f, -0.54f, 0.82f));

	model = glm::scale(model, glm::vec3(0.0100093f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	tennisBall.Draw(shader);
}

void renderSoccerBall(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-0.989999f, -0.21f, 0.2));

	model = glm::scale(model, glm::vec3(0.0710093f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	soccerBall.Draw(shader);
}

void renderDoll(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(13.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-183.5f), glm::vec3(1, 0, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-0.34f, 0.54f, -0.17f));

	model = glm::scale(model, glm::vec3(0.00100932));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	barbieDoll.Draw(shader);
}

void renderPony(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-19.5f), glm::vec3(0, 1, 0));
	//model = glm::translate(glm::mat4(model), glm::vec3(-0.44f, -0.55f, -0.75f));
	model = glm::translate(glm::mat4(model), glm::vec3(-0.44f, -0.55f, -0.939999f));

	model = glm::scale(model, glm::vec3(0.0310093f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	pony.Draw(shader);
}

void renderToyPlane(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(171.5f), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(0.979999f, -0.06f, -0.06f));

	model = glm::scale(model, glm::vec3(0.0110093f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	toyPlane.Draw(shader);
}

void renderMovingPlane(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));

	if (movePlane == true)
		model = glm::rotate(model, glm::radians(anglePlane), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-31.0f), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(xPlane, yPlane, zPlane));

	model = glm::scale(model, glm::vec3(0.22601f));
	
	if (movePlane == true) {
		if (xPlane <= -0.5 && yPlane <= 0.11)
		{
			xPlane += 0.01f;
			yPlane += 0.01f;
		}
		else if (xPlane <= -0.130001f && anglePlane >= -36.5) {
			xPlane += 0.01f;
			yPlane += 0.005f;
			anglePlane -= 0.05f;
		}
		else if (anglePlane >= -127.1f) {

			anglePlane -= 0.7f;
			yPlane += 0.0003f;

			if (xPlane <= 0.219999f) {
				xPlane += 0.001f;
			}
		}
		else if (yPlane >= -0.567f) {
			anglePlane -= 0.6f;
			yPlane -= 0.0007f;
		}
		else {
			movePlane = false;
			stopMoving = true;
		}
	}

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	movingPlane.Draw(shader);
}

void renderDogToy(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send teapot model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(63.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-92.5f), glm::vec3(1, 0, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(0.23, 1.17, -0.5f));

	model = glm::scale(model, glm::vec3(0.00600933f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw teapot
	dogToy.Draw(shader);
}

void renderPonyHouse(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	//model = glm::translate(glm::mat4(model), glm::vec3(-0.03f, -0.56f, -1.23f));
	model = glm::translate(glm::mat4(model), glm::vec3(-0.03f, -0.56f, -1.34f));

	model = glm::scale(model, glm::vec3(0.00300932f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	ponyHouse.Draw(shader);
}

void renderPaperDoll(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-235.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-183.0f), glm::vec3(1, 0, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-0.359999f, 0.539999f, -0.36f));

	model = glm::scale(model, glm::vec3(0.00600933f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	paperDoll.Draw(shader);
}

void renderCatToy(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(71.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-91.5f), glm::vec3(1, 0, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-0.17f, 0.969999f, -0.44f));

	model = glm::scale(model, glm::vec3(0.0120093f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	catToy.Draw(shader);
}

void renderFigurine(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(0.4f, -0.52f, -0.26f));

	model = glm::scale(model, glm::vec3(1.65903f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	legoFigurine.Draw(shader);
}

void renderNumberedDice(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(52.0f), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(1.12f, -0.55f, -0.05f));

	model = glm::scale(model, glm::vec3(0.0120093f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	numberedDice.Draw(shader);
}


void renderTruckToy(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(143.5f), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(0.85f, 0.1105f, -0.54f));

	model = glm::scale(model, glm::vec3(0.0700093f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	truckToy.Draw(shader);
}

void renderFirstShelf(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-1.11f, 0.329999f, 0.79f));

	model = glm::scale(model, glm::vec3(0.454007f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	shelf.Draw(shader);
}

void renderSecondShelf(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-1.12f, 0.329999f, -1.15f));

	model = glm::scale(model, glm::vec3(0.454007f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	shelf.Draw(shader);
}

void renderPicture(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-1.06f, 0.329999f, 0.82f));

	model = glm::scale(model, glm::vec3(0.128009f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	picture.Draw(shader);
}

void renderFrame(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(179.5f), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-0.999999f, 0.339999f, 0.45f));

	model = glm::scale(model, glm::vec3(0.137009f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	frame.Draw(shader);
}

void renderBooks(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-176.0f), glm::vec3(0, 1, 0));

	model = glm::translate(glm::mat4(model), glm::vec3(0.96f, 0.339999f, 1.18f));

	model = glm::scale(model, glm::vec3(0.307009f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	books.Draw(shader);
}

void renderBalloon(gps::Shader shader, bool showMap) {
	// select active shader program
	shader.useShaderProgram();

	//send model matrix data to shader
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(-38.5f), glm::vec3(0, 1, 0));
	model = glm::translate(glm::mat4(model), glm::vec3(-0.47f, yBalloon, -1.21f));

	if (yBalloon >= 0.0f)
	{
		down = true;
		up = false;
	}

	if (down) {
		yBalloon -= 0.0001f;
	}

	if (up) {
		yBalloon += 0.0001f;
	}

	if (yBalloon <= -0.02f) {
		down = false;
		up = true;
	}

	model = glm::scale(model, glm::vec3(0.0100093f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	//send normal matrix data to shader
	if (!showMap)
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// draw model
	balloon.Draw(shader);
}



void openingScene() {

	if (moveForward <= 50 && startOpeningScene == true) {
		moveForward += 1.0f;
		if (moveForward >= 10) {
			myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
			view = myCamera.getViewMatrix();
			myBasicShader.useShaderProgram();
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		}
	}
	else {
		if (angle <= 360 && startOpeningScene == true) {
			angle += 1.0f;
			model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		}
		else
			if (moveForward <= 100 && startOpeningScene == true)
			{
				moveForward += 1.0f;
				myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
				view = myCamera.getViewMatrix();
				myBasicShader.useShaderProgram();
				glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			}
			else
				startOpeningScene = false;
	}
}

void drawObjects(gps::Shader shader, bool showMap) {

	// render the models
	renderRoom(shader, showMap);
	renderMovingPlane(shader, showMap);
	renderRug(shader, showMap);
	renderBike(shader, showMap);
	renderMug(shader, showMap);
	renderBalloon(shader, showMap);
	renderRacket(shader, showMap);
	renderToyPlane(shader, showMap);
	renderFox(shader, showMap);
	renderPony(shader, showMap);
	renderDogToy(shader, showMap);
	renderSled(shader, showMap);
	renderTennisBall(shader, showMap);
	renderDoll(shader, showMap);
	renderDollHouse(shader, showMap);
	renderSoccerBall(shader, showMap);
	renderPonyHouse(shader, showMap);
	renderCrayons(shader, showMap);
	renderPaperDoll(shader, showMap);
	renderCatToy(shader, showMap);
	renderFigurine(shader, showMap);
	renderNumberedDice(shader, showMap);
	renderTruckToy(shader, showMap);
	renderFirstShelf(shader, showMap);
	renderSecondShelf(shader, showMap);
	renderPicture(shader, showMap);
	renderFrame(shader, showMap);
	renderBooks(shader, showMap);
}

void renderWithShadowMapping() {
	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	drawObjects(depthMapShader, true);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the C key

	if (showDepthMap) {
		glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myBasicShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::mat4(1.0f);
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myBasicShader, false);
	}
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render the scene
	renderWithShadowMapping();

	// render the skybox
	mySkyBox.Draw(skyboxShader, view, projection);

	// get point light and spot light positions
	getPointLightPos();
	getSpotLightPos();

	// start openin scene animation
	openingScene();

}

void cleanup() {
	myWindow.Delete();
	//cleanup code for your own data
}

int main(int argc, const char* argv[]) {

	try {
		initOpenGLWindow();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
	initFBO();
	setWindowCallbacks();

	glCheckError();

	std::vector<const GLchar*> faces;
	faces.push_back("skybox/right.tga");
	faces.push_back("skybox/left.tga");
	faces.push_back("skybox/up.tga");
	faces.push_back("skybox/down.tga");
	faces.push_back("skybox/back.tga");
	faces.push_back("skybox/front.tga");

	mySkyBox.Load(faces);
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f),
		(float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
		0.1f, 20.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));

	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

	return EXIT_SUCCESS;
}
