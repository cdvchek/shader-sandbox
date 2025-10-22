#include <glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include "Shader.hpp"

class CameraTest1 {
	public:
	glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
	float yaw = -90.0f;
	float pitch = 0.0f;
	float lastX = 400;
	float lastY = 300;
	float deltaTime = 0.0f;	// Time between current frame and last frame
	float lastFrame = 0.0f; // Time of last frame
	bool firstMouse = true;
	float fov = 45;

	static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
	{
		// Retrieve the pointer you set earlier
		CameraTest1* instance = static_cast<CameraTest1*>(glfwGetWindowUserPointer(window));
		if (instance->firstMouse)
		{
			instance->lastX = xpos;
			instance->lastY = ypos;
			instance->firstMouse = false;
		}
	
		float xoffset = xpos - instance->lastX;
		float yoffset = instance->lastY - ypos; 
		instance->lastX = xpos;
		instance->lastY = ypos;

		float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		instance->yaw   += xoffset;
		instance->pitch += yoffset;

		if(instance->pitch > 89.0f)
			instance->pitch = 89.0f;
		if(instance->pitch < -89.0f)
			instance->pitch = -89.0f;

		glm::vec3 direction;
		direction.x = cos(glm::radians(instance->yaw)) * cos(glm::radians(instance->pitch));
		direction.y = sin(glm::radians(instance->pitch));
		direction.z = sin(glm::radians(instance->yaw)) * cos(glm::radians(instance->pitch));
		instance->cameraFront = glm::normalize(direction);
	}  
 
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		CameraTest1* instance = static_cast<CameraTest1*>(glfwGetWindowUserPointer(window));
		instance->fov -= (float)yoffset;
		if (instance->fov < 1.0f)
			instance->fov = 1.0f;
		if (instance->fov > 45.0f)
			instance->fov = 45.0f; 
	}	

	static void processInput(GLFWwindow *window, CameraTest1 *thisObject)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		float cameraSpeed = 2.5f * thisObject->deltaTime;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			thisObject->cameraPos += cameraSpeed * thisObject->cameraFront;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			thisObject->cameraPos -= cameraSpeed * thisObject->cameraFront;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			thisObject->cameraPos -= glm::normalize(glm::cross(thisObject->cameraFront, thisObject->cameraUp)) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			thisObject->cameraPos += glm::normalize(glm::cross(thisObject->cameraFront, thisObject->cameraUp)) * cameraSpeed;
		
	}


	static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}  

	CameraTest1() {
		// Init Window Boilerplate //
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

		GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return;
		}
		glfwMakeContextCurrent(window);

		// Initialize GLAD
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return;
		}    

		// Open the window
		glViewport(0, 0, 800, 600);

		// If the window is resized, we call this.
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  

		
		// Load & set up the textures
		stbi_set_flip_vertically_on_load(true);
		unsigned int crateTexture, faceTexture;
		glGenTextures(1, &crateTexture);  
		glBindTexture(GL_TEXTURE_2D, crateTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		int width, height, nrChannels;
		// note: it loads from your working directory, so do it relative to where your makefile is.
		unsigned char *data = stbi_load("assets/container.png", &width, &height, &nrChannels, 0); 
		if (data)
		{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
				std::cout << "Failed to load crate texture" << std::endl;
		}
		stbi_image_free(data);	

		glGenTextures(1, &faceTexture);	
		glBindTexture(GL_TEXTURE_2D, faceTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		unsigned char *data2 = stbi_load("assets/happyface.png", &width, &height, &nrChannels, 0); 
		if (data2)
		{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
				glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
				std::cout << "Failed to load texture 2" << std::endl;
		}
		stbi_image_free(data2);	

		// Compile & Link shaders
		const char *vertexShaderSource = 
		"#version 330 core\n" 
		"layout (location = 0) in vec3 aPos;\n" 
		"layout (location = 1) in vec2 aTexCoord;\n"
		"out vec4 vertexPosition;\n"
		"out vec2 texCoord;\n"
		"	uniform mat4 model;\n"
		"	uniform mat4 view;\n"
		"	uniform mat4 projection;\n"
		"void main()\n" "{\n" 
		" gl_Position = projection * view * model * vec4(aPos, 1.0);\n" 
		" vertexPosition = gl_Position;\n"
		" texCoord = aTexCoord;\n"
		"}\0";

		const char *fragmentShaderSource = 
		"#version 330 core\n"
		"// fragment shader requires this one output variable, use out to say \"output\n"
		"out vec4 FragColor;\n" 
		"in vec4 vertexPosition; \n"
		"in vec2 texCoord;\n"
		"uniform sampler2D texture1;\n"
		"uniform sampler2D texture2;\n"
		"void main() { FragColor = mix(texture(texture1, texCoord), texture(texture2, texCoord), 0.2f); \n"
		" } \0";
		
		// Compilation!
		unsigned int vertexShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		
		int success; 
		char infoLog[512]; 
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
			
		if(!success) { 
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog); 
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl; 
		}

		unsigned int fragmentShader;
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		unsigned int shaderProgram;
		shaderProgram = glCreateProgram();

		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		glUseProgram(shaderProgram);
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
		}
		unsigned int VAO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
			
		float vertices[] = {
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
			0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

			0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
			0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
		};
		glm::vec3 cubePositions[] = {
			glm::vec3( 0.0f,  0.0f,  0.0f), 
			glm::vec3( 2.0f,  5.0f, -15.0f), 
			glm::vec3(-1.5f, -2.2f, -2.5f),  
			glm::vec3(-3.8f, -2.0f, -12.3f),  
			glm::vec3( 2.4f, -0.4f, -3.5f),  
			glm::vec3(-1.7f,  3.0f, -7.5f),  
			glm::vec3( 1.3f, -2.0f, -2.5f),  
			glm::vec3( 1.5f,  2.0f, -2.5f), 
			glm::vec3( 1.5f,  0.2f, -1.5f), 
			glm::vec3(-1.3f,  1.0f, -1.5f)  
		};
		unsigned int VBO;
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Texture coordinates.
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
		glEnableVertexAttribArray(1);

		glBindVertexArray(VAO);

		glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
		glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);

		glEnable(GL_DEPTH_TEST);

		CameraTest1* cam = this; // your instance

		// Associate your instance with the window
		glfwSetWindowUserPointer(window, cam);

		// Mouse input. set these once.
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);

		unsigned int lightVAO;
		glGenVertexArrays(1, &lightVAO);
		glBindVertexArray(lightVAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Set up lighting
		Shader lightShader = Shader("src/Shaders/lightingVert.vert", "src/Shaders/lightingFrag.frag");
		lightShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		lightShader.setVec3("lightColor",  1.0f, 1.0f, 1.0f);

		// Render loop
		while(!glfwWindowShouldClose(window))
		{
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;  
			processInput(window, this);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, crateTexture);
			glActiveTexture(GL_TEXTURE1);		
			glBindTexture(GL_TEXTURE_2D, faceTexture);

			glUseProgram(shaderProgram);

			// Rendering commands.
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glm::mat4 view;
			view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
			
			glm::mat4 projection; 
			projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);  

			int viewLoc = glGetUniformLocation(shaderProgram, "view");
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			
			int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
			
			glBindVertexArray(VAO);
			for(unsigned int i = 0; i < 10; i++)
			{
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, cubePositions[i]);
				float angle = 20.0f * i; 
				model = glm::rotate(model, glm::radians(angle * (float)glfwGetTime()), glm::vec3(1.0f, 0.3f, 0.5f));

				int modelLoc = glGetUniformLocation(shaderProgram, "model");
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}		

			lightShader.use();
			glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0, 0, 0));

			glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glBindVertexArray(lightVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			// Poll events and run them
			glfwPollEvents();   
			// 2 buffers. we load next frame into the back buffer, then swap it to the front
			// when it's done. This prevents silly artifacts and stuff.
			glfwSwapBuffers(window); 
		}

		glfwTerminate();
		return;
	}
};