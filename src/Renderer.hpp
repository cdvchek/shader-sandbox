#pragma once
#include "inspector.hpp"
#include <GLFW/glfw3.h>
#include <filesystem>
#include <glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>
#include <iostream>
#include <math.h>
#include <stb_image.h>
#include <string>

class Renderer {
public:
  std::string myTextInput = "";
  glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
  glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  float viewportWidth = 800;
  float viewportHeight = 600;
  float yaw = -90.0f;
  float pitch = 0.0f;
  float lastX = 400;
  float lastY = 300;
  float deltaTime = 0.0f; // Time between current frame and last frame
  float lastFrame = 0.0f; // Time of last frame
  bool firstMouse = true;
  bool disableInspector = true;
  float fov = 45;
  Inspector *inspector;

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
    ImGui_ImplOpenGL3_Init("#version 330");
  }

  void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    // Retrieve the pointer you set earlier
    if (!disableInspector)
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

  void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (!disableInspector)
      return;
    fov -= (float)yoffset;
    if (fov < 1.0f)
      fov = 1.0f;
    if (fov > 45.0f)
      fov = 45.0f;
  }

  void onEnterVisualizer(GLFWwindow *window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    inspector->refreshShaders();
  }

  void onExitVisualizer(GLFWwindow *window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    firstMouse = true;
  }

  void processInput(GLFWwindow *window) {
    static bool pressingTab = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
      if (!pressingTab) {
        pressingTab = true;
        disableInspector = !disableInspector;
        if (!disableInspector) {
          onExitVisualizer(window);
        } else {
          onEnterVisualizer(window);
        }
      }
    } else
      pressingTab = false;

    if (!disableInspector)
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

  void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
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

    // Input
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(
        window, [](GLFWwindow *window, int width, int height) {
          Renderer *self =
              static_cast<Renderer *>(glfwGetWindowUserPointer(window));
          self->framebuffer_size_callback(window, width, height);
        });
    glfwSetCursorPosCallback(
        window, [](GLFWwindow *window, double xpos, double ypos) {
          Renderer *self =
              static_cast<Renderer *>(glfwGetWindowUserPointer(window));
          self->mouse_callback(window, xpos, ypos);
        });
    glfwSetScrollCallback(
        window, [](GLFWwindow *window, double xoffset, double yoffset) {
          Renderer *self =
              static_cast<Renderer *>(glfwGetWindowUserPointer(window));
          self->scroll_callback(window, xoffset, yoffset);
        });

    return window;
  }

  static std::string getPath(const std::string &relativePath) {
    return std::filesystem::current_path().string() + "/" + relativePath;
  }

  void startRenderer() {
    std::cout << "Initializing Window..." << std::endl;
    GLFWwindow *window = initWindow();
    if (window == NULL) {
      std::cout << "Window Initialization error!" << std::endl;
      return;
    }

    inspector = new Inspector(window);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Model backpack =
        Model("assets/backpack/backpack.obj");

    std::cout << "Starting Render Loop" << std::endl;
    // Render loop
    while (!glfwWindowShouldClose(window)) {

      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;
      processInput(window);

      // Render the cubes.
      glm::mat4 view;
      glm::mat4 projection;
      glm::mat4 model = glm::rotate(
          glm::mat4(1.0f), glm::radians(-90.0f),
          glm::vec3(1, 0,
                    0)); // blender uses Z as up so we need to reorient him.
      model = glm::scale(model, glm::vec3(2));
      view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
      projection = glm::perspective(
          glm::radians(fov), viewportWidth / viewportHeight, 0.1f, 100.0f);

      auto lightingShader = inspector->shaders["lightSource"];
      lightingShader->use();
      lightingShader->setVec3("viewPos", cameraPos);
      lightingShader->setMat4("view", view);
      lightingShader->setMat4("projection", projection);
      lightingShader->setMat4("model", model);
      backpack.Draw(*lightingShader);

      if (!disableInspector) {
        inspector->drawInspector(viewportHeight);
      }

      glfwPollEvents();
      glfwSwapBuffers(window);
    }

    std::cout << "terminating" << std::endl;
    glfwTerminate();
    inspector->terminateInspector();
    return;
  }
};
