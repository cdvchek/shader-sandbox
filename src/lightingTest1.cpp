#include "Model.hpp"
#include "Shader.hpp"
#include "runLightingTest1.hpp"
#include <GLFW/glfw3.h>
#include <filesystem>
#include <fstream>
#include <glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>
#include <iostream>
#include <math.h>
#include <sstream>
#include <stb_image.h>
#include <string>
#include <variant>
#define NUM_POINT_LIGHTS 4

using UniformValue = std::variant<int, float, glm::vec3, glm::vec4>;
enum class UniformType { NoType, Int, Float, Vec3, Vec4, UniformRef };
struct Uniform {
  std::string name;
  std::string shaderName;
  UniformType type;
  UniformValue value;
  bool wasUniformRef = false;
};

std::string myTextInput = "";
std::map<std::string, std::map<std::string, Uniform>> uniforms;
unique_ptr<Shader> lightingShader = nullptr;
unique_ptr<Shader> lightCubeShader = nullptr;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 baseLightPos = glm::vec3(2.5f, 2.5f, 2.5f);
float viewportWidth = 800;
float viewportHeight = 600;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400;
float lastY = 300;
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
bool firstMouse = true;
bool enableVisualizerControl = true;
float fov = 45;
float lightingTestVertices[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.5f,  -0.5f,
    -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,
    0.0f,  -1.0f, 1.0f,  1.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
    1.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,

    -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.5f,  -0.5f,
    0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,
    0.0f,  1.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    1.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

    -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,
    -0.5f, -1.0f, 0.0f,  0.0f,  1.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f,
    0.0f,  0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
    0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f,  0.0f,
    -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,

    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,
    -0.5f, 1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,
    0.0f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
    0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.5f,  -0.5f,
    -0.5f, 0.0f,  -1.0f, 0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,
    -1.0f, 0.0f,  1.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
    1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,

    -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,
    -0.5f, 0.0f,  1.0f,  0.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,
    1.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f};
glm::vec3 pointLightPositions[] = {glm::vec3(0.7f, 0.2f, 2.0f),
                                   glm::vec3(0, 2, 0), glm::vec3(0, -2, 0),
                                   glm::vec3(2, -2.5, -2)};
glm::vec3 cubePositions[] = {

    glm::vec3(0, -1, 0), glm::vec3(0, 1, 0), glm::vec3(1, -1, 1)};

bool initUniformValue(Uniform *uniform, const Shader *shader,
                      std::istringstream &ss) {
  // Set value
  switch (uniform->type) {
  case UniformType::NoType:
    std::cout << "Not a valid type! " << std::endl;
    return false; // failure
  case UniformType::Int: {
    int intValue;
    ss >> intValue;
    shader->setInt(uniform->name, intValue);
    uniform->value = intValue;
    break;
  }
  case UniformType::Float: {
    float floatValue;
    ss >> floatValue;
    shader->setFloat(uniform->name, floatValue);
    uniform->value = floatValue;
    break;
  }
  case UniformType::Vec3: {
    float vec3Value1, vec3Value2, vec3Value3;
    ss >> vec3Value1;
    ss >> vec3Value2;
    ss >> vec3Value3;
    glm::vec3 vec3(vec3Value1, vec3Value2, vec3Value3);
    shader->setVec3(uniform->name, vec3);
    uniform->value = vec3;
    break;
  }
  case UniformType::Vec4: {
    float vec4Value1, vec4Value2, vec4Value3, vec4Value4;
    ss >> vec4Value1;
    ss >> vec4Value2;
    ss >> vec4Value3;
    ss >> vec4Value4;
    glm::vec4 vec4(vec4Value1, vec4Value2, vec4Value3, vec4Value4);
    shader->setVec4(uniform->name, vec4Value1, vec4Value2, vec4Value3,
                    vec4Value4);
    uniform->value = vec4;
    break;
  }
  case UniformType::UniformRef: {
    // Later, I'd like to defer this so you don't need to have all your uniform
    // refs at the end of the file
    std::string shaderName;
    ss >> shaderName;
    std::string refName;
    ss >> refName;

    // Uniform Refs need to be placed AFTER the uniform they're referencing. If
    // not, we need to warn them it's a bad file
    if (uniforms.count(shaderName) == 0) {
      std::cout << "bad shader name!: " << shaderName << std::endl;
      return false;
    } else if (uniforms[shaderName].count(refName) == 0) {
      std::cout << "Uniform ref needs to be placed AFTER the uniform it's "
                   "referencing!"
                << std::endl;
      return false;
    }

    Uniform ref = uniforms[shaderName][refName];
    uniform->value = ref.value;
    uniform->type = ref.type;
    uniform->wasUniformRef = true;
    switch (ref.type) {
    case UniformType::Int:
      shader->setInt(uniform->name, std::get<int>(ref.value));
      break;
    case UniformType::Float:
      shader->setFloat(uniform->name, std::get<float>(ref.value));
      break;
    case UniformType::Vec3:
      shader->setVec3(uniform->name, std::get<glm::vec3>(ref.value));
      break;
    case UniformType::Vec4:
      shader->setVec4(uniform->name, std::get<glm::vec4>(ref.value));
      break;
    }
    break;
  }
  }
  return true;
}

void readUniforms(const Shader *shader, const char *path) {
  shader->use();

  std::ifstream file(path);
  if (!file.is_open()) {
    std::cout << "failed to open uniforms file " << path << std::endl;
    return;
  }
  std::string line;
  std::string shaderName;
  {
    std::getline(file, line);
    std::istringstream ss(line);

    if (!(ss >> shaderName) || shaderName == "") {
      std::cerr << "Failed to read shader name from top of file!" << line
                << std::endl;
      return;
    }

    // Check if there’s any extra text
    std::string extra;
    if (ss >> extra) {
      std::cerr << "Unexpected extra text on top line: " << line << std::endl;
      return;
    }

    if (uniforms.count(shaderName) > 0) {
      std::cout << "Error: duplicate shader names!" << line << std::endl;
      return;
    }
  }
  while (std::getline(file, line)) {
    // Types: Int, Float, Vec3
    if (line.empty() || line[0] == '#')
      continue;
    std::istringstream ss(line);
    std::string typeStr;
    Uniform uniform;
    uniform.shaderName = shaderName;
    ss >> typeStr;
    ss >> uniform.name;

    if (uniforms.count(shaderName) > 0 &&
        uniforms[shaderName].count(uniform.name) > 0) {
      std::cout << "duplicate name! " << uniform.name << std::endl;
    }

    // Parse uniform type from first word
    char firstChar = typeStr[0];
    switch (firstChar) {
    case 'I':
      if (typeStr == "Int")
        uniform.type = UniformType::Int;
      break;
    case 'F':
      if (typeStr == "Float")
        uniform.type = UniformType::Float;
      break;
    case 'V':
      if (typeStr == "Vec3")
        uniform.type = UniformType::Vec3;
      else if (typeStr == "Vec4") {
        uniform.type = UniformType::Vec4;
      }
      break;
    case 'U':
      if (typeStr == "Uniform")
        uniform.type = UniformType::UniformRef;
    }

    bool success = initUniformValue(&uniform, shader, ss);
    if (ss.fail() || !success) {
      std::cout << "failed to read line: " << line << std::endl;
    } else {
      uniforms[uniform.shaderName][uniform.name] = uniform;
    }
  }
}

void initShaders() {
  uniforms.clear();
  std::cout << "loading shaders..." << std::endl;
  lightingShader = std::make_unique<Shader>("src/Shaders/lightingVert.vert",
                                            "src/Shaders/multipleLights.frag");

  lightingShader->use();
  readUniforms(lightingShader.get(), "src/uniforms/multipleLights.txt");

  lightCubeShader = std::make_unique<Shader>("src/Shaders/lightCubeVert.vert",
                                             "src/Shaders/lightCube.frag");
  lightCubeShader->use();
  readUniforms(lightCubeShader.get(), "src/uniforms/lightCube.txt");
  // lightCubeShader->setVec3("objectColor", .5f, 1.0f, .5f);
  // lightCubeShader->setVec3("lightColor",  .5f, 1.0f, .5f);

  for (const auto &[shaderName, shaderUniforms] : uniforms) {
    std::cout << "Shader: " << shaderName << std::endl;

    for (const auto &[uniformName, uniform] : shaderUniforms) {
      std::cout << "  " << uniformName << " (type: ";
      if (uniform.wasUniformRef)
        std::cout << " Uniform Ref -> ";
      switch (uniform.type) {
      case UniformType::Int:
        std::cout << "Int) = " << std::get<int>(uniform.value);
        break;
      case UniformType::Float:
        std::cout << "Float) = " << std::get<float>(uniform.value);
        break;
      case UniformType::Vec3: {
        auto v = std::get<glm::vec3>(uniform.value);
        std::cout << "Vec3) = (" << v.x << ", " << v.y << ", " << v.z << ")";
        break;
      }
      case UniformType::Vec4: {
        auto v = std::get<glm::vec4>(uniform.value);
        std::cout << "Vec4) = (" << v.x << ", " << v.y << ", " << v.z << ", "
                  << v.w << ")";
        break;
      }
      case UniformType::UniformRef:
        std::cout << "UniformRef)";
        break;
      default:
        std::cout << "Unknown)";
        break;
      }

      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
}

static void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  // Retrieve the pointer you set earlier
  if (!enableVisualizerControl)
    return;
  if (firstMouse) {
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

  yaw += xoffset;
  pitch += yoffset;

  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

  glm::vec3 direction;
  direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.y = sin(glm::radians(pitch));
  direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(direction);
}

static void scroll_callback(GLFWwindow *window, double xoffset,
                            double yoffset) {
  if (!enableVisualizerControl)
    return;
  fov -= (float)yoffset;
  if (fov < 1.0f)
    fov = 1.0f;
  if (fov > 45.0f)
    fov = 45.0f;
}

void onEnterVisualizer(GLFWwindow *window) {
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  initShaders();
}

void onExitVisualizer(GLFWwindow *window) {
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  firstMouse = true;
}

static void processInput(GLFWwindow *window) {
  static bool pressingTab = false;
  if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
    if (!pressingTab) {
      pressingTab = true;
      enableVisualizerControl = !enableVisualizerControl;
      if (!enableVisualizerControl) {
        onExitVisualizer(window);
      } else {
        onEnterVisualizer(window);
      }
    }
  } else
    pressingTab = false;

  if (!enableVisualizerControl)
    return;

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  float cameraSpeed = 2.5f * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -=
        glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos +=
        glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    cameraPos += glm::normalize(cameraUp) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    cameraPos -= glm::normalize(cameraUp) * cameraSpeed;
}

static void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
  glViewport(0, 0, width, height);
  viewportHeight = height;
  viewportWidth = width;
}

GLFWwindow *initWindow() {
  // Init Window Boilerplate //
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *window = glfwCreateWindow(viewportWidth, viewportHeight,
                                        "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return NULL;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
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

void setupImGui(GLFWwindow *window) {
  // 1. Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  // 2. Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // 3. Initialize ImGui backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330"); // matches your GL version
}

void setImGUIText() {}

void runLightingTest1() {
  GLFWwindow *window = initWindow();
  if (window == NULL)
    return;

  setupImGui(window);

  unsigned int lightCubeVAO, VBO;

  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &lightCubeVAO);

  glBindVertexArray(lightCubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(lightingTestVertices),
               lightingTestVertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // Load & set up the textures
  stbi_set_flip_vertically_on_load(true);

  Model backpack = Model("assets/backpack/backpack.obj");

  initShaders();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Render loop
  while (!glfwWindowShouldClose(window)) {

    // Start new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Left panel (like Unity Inspector)
    ImGui::SetNextWindowPos(ImVec2(10, 10),
                            ImGuiCond_FirstUseEver); // top-left corner
    ImGui::SetNextWindowSize(ImVec2(250, viewportHeight - 20),
                             ImGuiCond_FirstUseEver);

    ImGui::Begin("Inspector", nullptr,
                 ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::Text("Object Properties");

    // Add a text input
    char buffer[256];
    strncpy(buffer, myTextInput.c_str(), sizeof(buffer));
    buffer[sizeof(buffer) - 1] = 0;
    if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
      myTextInput = std::string(buffer);
    }

    ImGui::End();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    processInput(window);

    pointLightPositions[0] =
        glm::vec3(baseLightPos.x * sin(glfwGetTime()),
                  baseLightPos.y * cos(glfwGetTime() + 3.141592),
                  baseLightPos.z * cos(glfwGetTime()));
    pointLightPositions[3] = glm::vec3(baseLightPos.x * cos(glfwGetTime()),
                                       baseLightPos.y * sin(glfwGetTime()),
                                       baseLightPos.z * sin(glfwGetTime()));

    // Render the cubes.
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 model = glm::rotate(
        glm::mat4(1.0f), glm::radians(-90.0f),
        glm::vec3(1, 0, 0)); // blender uses Z as up so we need to reorient him.
    model = glm::scale(model, glm::vec3(2));
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    projection = glm::perspective(glm::radians(fov),
                                  viewportWidth / viewportHeight, 0.1f, 100.0f);

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

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Poll events and run them
    glfwPollEvents();
    // 2 buffers. we load next frame into the back buffer, then swap it to the
    // front when it's done. This prevents silly artifacts and stuff.
    glfwSwapBuffers(window);
  }

  std::cout << "terminating" << std::endl;
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwTerminate();
  return;
}
