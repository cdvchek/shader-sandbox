#include "window.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

unsigned int texture1, texture2;

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
}  

void drawTriangle() {
	// this is just the text for the shader 
	const char *vertexShaderSource = "#version 330 core\n" 
	"layout (location = 0) in vec3 aPos;\n" 
	"void main()\n" "{\n" 
	" gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n" 
	"}\0";

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

	const char *fragmentShaderSource = 
	"#version 330 core\n"
	"// fragment shader requires this one output variable, use out to say \"output\n"
	"out vec4 FragColor;\n" 
	"// orange, opaque\n"
	"void main() { FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f); } \0";
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(shaderProgram);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,
			0.0f,  0.5f, 0.0f
	}; 

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void drawRectangle() {
	// this is just the text for the shader 
	const char *vertexShaderSource = 
	"#version 330 core\n" 
	"layout (location = 0) in vec3 aPos;\n" 
	"layout (location = 1) in vec3 aColor;\n"
	"layout (location = 2) in vec2 aTexCoord;\n"
	"out vec4 vertexColor;\n"
	"out vec4 vertexPosition;\n"
	"out vec2 texCoord;\n"
	"uniform mat4 transform;\n"
	"void main()\n" "{\n" 
	" gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n" 
	" vertexColor = vec4(aColor, 1.0f);\n"
	" vertexPosition = gl_Position;\n"
	" texCoord = aTexCoord;\n"
	"}\0";

	const char *fragmentShaderSource = 
	"#version 330 core\n"
	"// fragment shader requires this one output variable, use out to say \"output\n"
	"out vec4 FragColor;\n" 
	"in vec4 vertexColor; \n"
	"in vec4 vertexPosition; \n"
	"in vec2 texCoord;\n"
	"uniform sampler2D texture1;\n"
	"uniform sampler2D texture2;\n"
	"uniform vec4 color2;\n"
	"void main() { FragColor = mix(texture(texture1, texCoord), texture(texture2, texCoord), 0.2f); \n"
	//"void main() { FragColor = texture(texture2, texCoord); \n"

	"	if (vertexPosition.x > 0) { FragColor = mix(color2 * vertexColor, texture(texture2, texCoord), 0.2f); }\n"
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

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
	}

	float timeValue = glfwGetTime();
	float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
	int vertexColor2Location = glGetUniformLocation(shaderProgram, "color2");
	glUseProgram(shaderProgram);
	glUniform4f(vertexColor2Location, 0.0f, greenValue, 0.0f, 1.0f);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(shaderProgram);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	float vertices[] = {
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
	}; 
	unsigned int indices[] = {
		0, 1, 2, // first triangle
		0, 2, 3 // second
	};
	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	// Texture coordinates.
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(sizeof(float) * 6));
	glEnableVertexAttribArray(2);

	glBindVertexArray(VAO);

	glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);

	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::translate(trans, glm::vec3(sin((float)glfwGetTime())/2, cos((float)glfwGetTime())/2, 0.0f));
	trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f)); 
	unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// Lots of bad comments that are just code duplication because I'm learning right now.
int Window() {
	// Init Window
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
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}    

	// Open the window
	glViewport(0, 0, 800, 600);

	// If the window is resized, we call this.
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  

	stbi_set_flip_vertically_on_load(true);
	// set the texture wrapping/filtering options (on the currently bound texture object)	
	glGenTextures(1, &texture1);  
	glGenTextures(1, &texture2);  

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);
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
			std::cout << "Failed to load texture 2" << std::endl;
	}
	stbi_image_free(data);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);
	
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

	// Render loop
	while(!glfwWindowShouldClose(window))
	{
		processInput(window);

		// Rendering commands.
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		drawRectangle();

		// Poll events and run them
		glfwPollEvents();   
		// 2 buffers. we load next frame into the back buffer, then swap it to the front
		// when it's done. This prevents silly artifacts and stuff.
		glfwSwapBuffers(window); 
	}

	glfwTerminate();
	return 0;
}
