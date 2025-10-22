#include <glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include "model.hpp"
#include "Shader.hpp"
#include "runLightingTest1.hpp"

#define NUM_POINT_LIGHTS 4
Shader* lightingShader = nullptr;
Shader* lightCubeShader = nullptr;
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
glm::vec3 baseLightPos	= glm::vec3(2.5f, 2.5f, 2.5f);
float viewportWidth = 800;
float viewportHeight = 600;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400;
float lastY = 300;
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
bool firstMouse = true;
float fov = 45;
float lightingTestVertices[] = {
	// positions          // normals           // texture coords
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
};
glm::vec3 pointLightPositions[] = {
	glm::vec3( 0.7f,  0.2f,  2.0f),
	glm::vec3( 0, 3, 0),
	glm::vec3(0, -3, 0),
	glm::vec3( 2,  -2.5, -2)
};  
glm::vec3 cubePositions [] = {
	glm::vec3( 1, 1, -1),
	glm::vec3( 0, -1, 0),
	glm::vec3( 0, 1, 0),
	glm::vec3( 1, -1, 1)
};

void readUniforms(Shader* shader, const char* path) {
	enum UniformType {
		NoType,
		Int,
		Float,
		Vec3,
		Vec4,
	};

	shader->use();

	std::ifstream file(path);
	if (!file.is_open()) {
		std::cout << "failed to open uniforms file " << path << std::endl;
		return;
	}
	std::string line;
	while (std::getline(file, line)) {
		// Types: Int, Float, Vec3
		if (line.empty() || line[0] == '#') continue;
		std::istringstream ss(line);
		std::string typeStr;
		std::string uniformName;
		ss >> typeStr;
		ss >> uniformName;
		char firstChar = typeStr[0];	
		UniformType type = UniformType::NoType;
		switch(firstChar) { 
			case 'I':
				if (typeStr == "Int") 
					type = UniformType::Int;
				break;
			case 'F':
				if (typeStr == "Float") 
					type = UniformType::Float;
				break;
			case 'V':
				if (typeStr == "Vec3") 
					type = UniformType::Vec3;
				else if (typeStr == "Vec4") {
					type = UniformType::Vec4;
				}
				break;
		}
		switch(type) {
			case UniformType::NoType:
				std::cout << "Not a valid type: " << typeStr << std::endl;
				break;
			case UniformType::Int:
				int intValue;
				ss >> intValue;
				shader->setInt(uniformName, intValue);
				break;
			case UniformType::Float:
				float floatValue;
				ss >> floatValue;
				shader->setFloat(uniformName, floatValue);
				break;
			case UniformType::Vec3:
				float vec3Value1, vec3Value2, vec3Value3;
				ss >> vec3Value1; ss >> vec3Value2; ss >> vec3Value3;
				shader->setVec3(uniformName, vec3Value1, vec3Value2, vec3Value3);
				break;
			case UniformType::Vec4:
				float vec4Value1, vec4Value2, vec4Value3, vec4Value4;
				ss >> vec4Value1; ss >> vec4Value2; ss >> vec4Value3; ss >> vec4Value4;
				shader->setVec4(uniformName, vec4Value1, vec4Value2, vec4Value3, vec4Value4);
				break;
		}
		if (ss.fail()) {
			std::cout << "failed to read " << uniformName << std::endl;
		}
	}
}

void initShaders() {
	std::cout << "loading shaders..." << std::endl;
	lightingShader = new Shader("src/Shaders/lightingVert.vert", "src/Shaders/cel.frag");


	lightingShader->use();
	readUniforms(lightingShader, "src/uniforms/multipleLights.txt");

	lightCubeShader = new Shader("src/Shaders/lightCubeVert.vert", "src/Shaders/lightCube.frag");
	lightCubeShader->use();
	// lightCubeShader->setVec3("objectColor", .5f, 1.0f, .5f);
	// lightCubeShader->setVec3("lightColor",  .5f, 1.0f, .5f);
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	// Retrieve the pointer you set earlier
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; 
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw   += xoffset;
	pitch += yoffset;

	if(pitch > 89.0f)
		pitch = 89.0f;
	if(pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}  

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov -= (float)yoffset;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 45.0f)
		fov = 45.0f; 
}	

static void processInput(GLFWwindow *window)
{
	static bool pressingR = false;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		if (!pressingR) {
			pressingR = true;
			initShaders();
		}
	}
	else pressingR = false;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	float cameraSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		cameraPos += glm::normalize(cameraUp) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		cameraPos -= glm::normalize(cameraUp) * cameraSpeed;
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	viewportHeight = height;
	viewportWidth = width;
}  

GLFWwindow* initWindow() {
	// Init Window Boilerplate //
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(viewportWidth, viewportHeight, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return NULL;
	}    

	// Open the window
	glViewport(0, 0, viewportWidth, viewportHeight);

	// If the window is resized, we call this.
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  

	glEnable(GL_DEPTH_TEST);
	
	// Associate your instance with the window
	// Mouse input. set these once.
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	return window;
}

static std::string getPath(const std::string &relativePath) {
	return std::filesystem::current_path().string() + "/" + relativePath;
}

void runLightingTest1() {
	GLFWwindow* window = initWindow();
	if (window == NULL) 
		return;

	unsigned int lightCubeVAO, VBO;

	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &lightCubeVAO);

	glBindVertexArray(lightCubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lightingTestVertices), lightingTestVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Load & set up the textures
	stbi_set_flip_vertically_on_load(true);

	Model backpack = Model("assets/backpack/backpack.obj");

	initShaders();
	glEnable(GL_DEPTH_TEST);

	// Render loop
	while(!glfwWindowShouldClose(window))
	{

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;  
		processInput(window);

		pointLightPositions[0] = glm::vec3(baseLightPos.x * sin(glfwGetTime()), 
				baseLightPos.y * cos(glfwGetTime() + 3.141592), baseLightPos.z * cos(glfwGetTime()));
		pointLightPositions[3] = glm::vec3(baseLightPos.x * cos(glfwGetTime()), 
				baseLightPos.y * sin(glfwGetTime()), baseLightPos.z * sin(glfwGetTime()));

		// Render the cubes.
		glm::mat4 view;
		glm::mat4 projection; 
		glm::mat4 model = glm::mat4(1.0f);
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		projection = glm::perspective(glm::radians(fov), viewportWidth / viewportHeight, 0.1f, 100.0f);  

		lightingShader->use();
		lightingShader->setVec3("viewPos", cameraPos); 
		lightingShader->setMat4("view", view);
		lightingShader->setMat4("projection", projection);
		lightingShader->setMat4("model", model);
		for (int i = 0; i < NUM_POINT_LIGHTS; i++) {
			std::stringstream s;
			s << "pointLights[" << i << "].position";
			lightingShader->setVec3(s.str(), pointLightPositions[i]);
		}

		backpack.Draw(*lightingShader);	

		// Light Sources
		lightCubeShader->use();
		lightCubeShader->setMat4("view", view);
		lightCubeShader->setMat4("projection", projection);
		glBindVertexArray(lightCubeVAO);

		// Draw 4 light sources
		for (int i = 0; i < NUM_POINT_LIGHTS; i++) {
			glm::mat4 cubeModel = glm::mat4(1.0f);
			cubeModel = glm::translate(cubeModel, pointLightPositions[i]);
			cubeModel = glm::scale(cubeModel, glm::vec3(0.2f)); 
			lightCubeShader->setMat4("model", cubeModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// Poll events and run them
		glfwPollEvents();   
		// 2 buffers. we load next frame into the back buffer, then swap it to the front
		// when it's done. This prevents silly artifacts and stuff.
		glfwSwapBuffers(window); 
	}

	std::cout << "terminating" << std::endl;
	glfwTerminate();
	return;
}
