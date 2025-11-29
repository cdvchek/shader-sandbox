#include "Model.hpp"
#include "Shader.hpp"
#include "nlohmanjson.hpp"
#include <GLFW/glfw3.h>
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
#include <stb_image.h>
#include <string>
#include <variant>

class Inspector {
public:
  using UniformValue = std::variant<int, float, glm::vec3, glm::vec4>;
  enum class UniformType { NoType, Int, Float, Vec3, Vec4, UniformRef };
  struct WorldObject {
    std::string shaderUsed;
    std::string objectName;
    std::string objectPosition;
  };
  struct UniformRef {
    std::string shaderName;
    std::string uniformName;
    std::string uniformType;
  };
  struct Uniform {
    std::string name;
    std::string shaderName;
    UniformType type;
    UniformValue value;
    bool wasUniformRef = false;
    UniformRef ref;
  };

  nlohmann::json json;
  std::string jsonPath;
  std::map<std::string, std::map<std::string, Uniform>> uniforms;
  std::vector<string> shaderPaths = {
      "src/Shaders/lightingVert.vert", "src/Shaders/multipleLights.frag",
      "src/Shaders/lightCubeVert.vert", "src/Shaders/lightCube.frag",
      "src/Shaders/cel.frag"};
  std::map<std::string, Shader *> shaders;
  std::map<std::string, WorldObject> objects;

  std::string newUniformName;
  std::string newUniformShaderName;
  UniformType newUniformType = UniformType::NoType;
  std::string newObjectName;
  std::string newObjectShaderUsed;

  Inspector(GLFWwindow *window) {
    // note: You must initialize GFLW before doing thise!!!!
    std::cout << "Initializing Inspector... Make sure you terminate it later"
              << std::endl;
    shaders.insert(std::make_pair(
        "litCube", new Shader(shaderPaths[0].c_str(), shaderPaths[1].c_str())));
    shaders.insert(
        std::make_pair("lightSource", new Shader(shaderPaths[2].c_str(),
                                                 shaderPaths[3].c_str())));
    jsonPath = "src/uniforms/project.json";

    std::fstream file(jsonPath,
                      std::fstream::in | std::fstream::out | std::fstream::app);

    if (!file.is_open()) {
      std::cout << "failed to open or create uniforms file " << jsonPath
                << std::endl;
      return;
    }

    if (file.peek() == std::ifstream::traits_type::eof()) {
      std::cout << "No uniforms initialized, making new json file" << std::endl;
      saveProjectJSON();
    }

    file.close();

    for (auto &[shaderName, shader] : shaders) {
      shader->use();
      loadProjectJSON(shaderName);
    }

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

  void terminateInspector() {
    std::cout << "Terminating Inspector..." << std::endl;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    saveProjectJSON();
  }

  bool initUniformValueFromJSON(Uniform *uniform, std::string shaderName) {
    // Set value
    Shader *shader = shaders[shaderName];
    switch (uniform->type) {
    case UniformType::NoType:
      std::cout << "Not a valid type! " << std::endl;
      return false; // failure
    case UniformType::Int: {
      int intValue =
          json["shaders"][shaderName]["uniforms"][uniform->name]["value"];
      shader->setInt(uniform->name, intValue);
      uniform->value = intValue;
      break;
    }
    case UniformType::Float: {
      std::cout << "initializing float " << uniform->name << std::endl;
      auto &val =
          json["shaders"][shaderName]["uniforms"][uniform->name]["value"];

      std::cout << "initializing " << val << std::endl;
      float floatValue = val.get<float>();
      shader->setFloat(uniform->name, floatValue);
      uniform->value = floatValue;
      break;
    }
    case UniformType::Vec3: {
      float vec3Value1, vec3Value2, vec3Value3;
      nlohmann::json valueJSON =
          json["shaders"][shaderName]["uniforms"][uniform->name]["value"];
      vec3Value1 = valueJSON[0];
      vec3Value2 = valueJSON[1];
      vec3Value3 = valueJSON[2];
      glm::vec3 vec3(vec3Value1, vec3Value2, vec3Value3);
      shader->setVec3(uniform->name, vec3);
      uniform->value = vec3;
      break;
    }
    case UniformType::Vec4: {
      float vec4Value1, vec4Value2, vec4Value3, vec4Value4;
      nlohmann::json valueJSON =
          json["shaders"][shaderName]["uniforms"][uniform->name]["value"];
      vec4Value1 = valueJSON[0];
      vec4Value2 = valueJSON[1];
      vec4Value3 = valueJSON[2];
      vec4Value4 = valueJSON[3];
      glm::vec4 vec4(vec4Value1, vec4Value2, vec4Value3, vec4Value4);
      shader->setVec4(uniform->name, vec4Value1, vec4Value2, vec4Value3,
                      vec4Value4);
      uniform->value = vec4;
      break;
    }
    case UniformType::UniformRef: {
      // Later, I'd like to defer this so you don't need to have all your
      // uniform refs at the end of the file
      nlohmann::json valueJSON =
          json["shaders"][shaderName]["uniforms"][uniform->name]["value"];
      std::string refShaderName = valueJSON["shaderName"];
      std::string refName = valueJSON["uniformName"];

      // Uniform Refs need to be placed AFTER the uniform they're referencing.
      // If not, we need to warn them it's a bad file
      if (uniforms.count(refShaderName) == 0) {
        std::cout << "bad shader name!: " << refShaderName << std::endl;
        return false;
      } else if (uniforms[refShaderName].count(refName) == 0) {
        std::cout << "Uniform ref needs to be placed AFTER the uniform it's "
                     "referencing!"
                  << std::endl;
        return false;
      }

      Uniform ref = uniforms[refShaderName][refName];
      uniform->value = ref.value;
      uniform->type = ref.type;
      switch (ref.type) {
        uniform->wasUniformRef = true;
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

  bool initUniformValue(Uniform *uniform, std::string shaderName) {
    // Set value
    Shader *shader = shaders[shaderName];
    UniformValue value = uniform->value;
    switch (uniform->type) {
    case UniformType::NoType:
      std::cout << "Not a valid type! " << std::endl;
      return false; // failure
    case UniformType::Int: {
      shader->setInt(uniform->name, std::get<int>(value));
      break;
    }
    case UniformType::Float: {
      shader->setFloat(uniform->name, std::get<float>(value));
      break;
    }
    case UniformType::Vec3: {
      shader->setVec3(uniform->name, std::get<glm::vec3>(value));
      break;
    }
    case UniformType::Vec4: {
      shader->setVec4(uniform->name, std::get<glm::vec4>(value));
      break;
    }
    case UniformType::UniformRef: {
      break;
    }
    }
    return true;
  }

  void refreshUniforms(const std::string shaderName) {
    Shader *shader = shaders[shaderName];
    shader->use();

    for (auto &[uniformName, uniform] : uniforms[shaderName]) {
      initUniformValue(&uniform, shaderName);
    }
  }

  void loadProjectJSON(const std::string shaderName) {
    Shader *shader = shaders[shaderName];
    shader->use();

    std::fstream file(jsonPath,
                      std::fstream::in | std::fstream::out | std::fstream::app);
    if (!file.is_open()) {
      std::cout << "failed to open or create uniforms file " << jsonPath
                << std::endl;
      return;
    }

    json = nlohmann::json::parse(file);
    std::cout << "json parsed, now loading uniforms" << std::endl;

    // see nlohmann json docs and lookup "structured bindings"
    for (auto &[uniformName, uniformJSON] :
         json["shaders"][shaderName]["uniforms"].items()) {
      Uniform uniform;
      uniform.shaderName = shaderName;
      uniform.name = uniformName;

      if (uniforms.count(shaderName) > 0 &&
          uniforms[shaderName].count(uniform.name) > 0) {
        std::cout << "duplicate name! " << uniform.name << std::endl;
      }

      uniform.type = uniformJSON["type"].get<UniformType>();

      std::cout << "loading " << uniform.name << std::endl;
      bool success = initUniformValueFromJSON(&uniform, shaderName);
      if (!success) {
        std::cout << "failed to read uniform: " << uniform.name << std::endl;
      } else {
        uniforms[uniform.shaderName][uniform.name] = uniform;
      }
    }
    file.close();
  }

  void saveProjectJSON() {
    json["shaders"] = {};
    for (auto &[shaderName, shader] : shaders) {
      json["shaders"][shaderName] = {{"vertex", shader->vertexPath},
                                     {"fragment", shader->fragmentPath}};
      json["shaders"][shaderName]["uniforms"] = nlohmann::json::object();

      for (auto &[uniformName, uniform] : uniforms[shaderName]) {
        json["shaders"][shaderName]["uniforms"][uniformName] =
            nlohmann::json::object({{"type", uniform.type},
                                    {"wasUniformRef", uniform.wasUniformRef}});
        switch (uniform.type) {
        case UniformType::Int: {
          int v = std::get<int>(uniform.value);
          json["shaders"][shaderName]["uniforms"][uniformName]["value"] = v;
          break;
        }
        case UniformType::Float: {
          float v = std::get<float>(uniform.value);
          json["shaders"][shaderName]["uniforms"][uniformName]["value"] = v;
          break;
        }
        case UniformType::Vec3: {
          glm::vec3 v = std::get<glm::vec3>(uniform.value);
          json["shaders"][shaderName]["uniforms"][uniformName]["value"] = {
              v.x, v.y, v.z};
          break;
        }
        case UniformType::Vec4: {
          glm::vec4 v = std::get<glm::vec4>(uniform.value);
          json["shaders"][shaderName]["uniforms"][uniformName]["value"] = {
              v.x, v.y, v.z, v.w};
          break;
        }
        case UniformType::UniformRef: {
          UniformRef ref = uniform.ref;
          json["shaders"][shaderName]["uniforms"][uniformName]["value"] = {
              {"shaderName", ref.shaderName},
              {"uniformName", ref.uniformName},
              {"type", ref.uniformType}};
          break;
        }
        }
      }
    }
    std::ofstream outfile(jsonPath);
    if (outfile.is_open()) {
      outfile << json.dump(4);
      outfile.close();
    }
  }

  void refreshShaders() {
    std::cout << "Initializing Shaders..." << std::endl;
    shaders.clear();
    shaders.insert(std::make_pair(
        "litCube", new Shader(shaderPaths[0].c_str(), shaderPaths[1].c_str())));
    shaders.insert(
        std::make_pair("lightSource", new Shader(shaderPaths[2].c_str(),
                                                 shaderPaths[3].c_str())));
    for (auto &[shaderName, shader] : shaders) {
      shader->use();
      refreshUniforms(shaderName);
    }

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

  void drawInspector(float viewportHeight) {
    // Start new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //
    ImGui::SetNextWindowPos(ImVec2(10, 10),
                            ImGuiCond_FirstUseEver); // top-left corner
    ImGui::SetNextWindowSize(ImVec2(200, viewportHeight - 20),
                             ImGuiCond_FirstUseEver);

    ImGui::Begin("Inspector", nullptr,
                 ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::Text("Object Properties");

    drawUniformEditors();
    ImGui::End();
    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  void drawTextInput(std::string *value, const char *label) {
    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "%s", value->c_str());
    if (ImGui::InputText(label, buffer, sizeof(buffer))) {
      *value = buffer;
    }
  }

  void drawAddUniformMenu() {
    ImGui::Text("Add Uniform");
    drawTextInput(&newUniformName, "Uniform Name");

    // Build a list of shader names
    std::vector<const char *> shaderChoices;
    shaderChoices.reserve(shaders.size() + 1);
    shaderChoices.push_back("");
    for (auto &[name, shader] : shaders) {
      shaderChoices.push_back(name.c_str());
    }
    // Keep track of the selected shader index
    static int shaderChoice = 0;
    if (newUniformShaderName == "")
      shaderChoice = 0;
    // Display combo box
    if (ImGui::Combo("Shader", &shaderChoice, shaderChoices.data(),
                     (int)shaderChoices.size())) {
      // When selection changes, update newUniformShaderName
      newUniformShaderName = shaderChoices[shaderChoice];
    }
    // Dropdown menu from enum
    const char *typeNames[6] = {"None", "Int",  "Float",
                                "Vec3", "Vec4", "Reference"};
    int choice = static_cast<int>(newUniformType);
    if (ImGui::Combo("Uniform Type", &choice, typeNames,
                     IM_ARRAYSIZE(typeNames))) {
      newUniformType = static_cast<UniformType>(choice);
    }

    if (ImGui::Button("Add Uniform", ImVec2(100, 20))) {
      bool uniqueValidUniform = false;
      bool validShaderName = shaders.count(newUniformShaderName) >= 1;
      if (validShaderName) {
        uniqueValidUniform =
            uniforms[newUniformShaderName].count(newUniformName) <= 0 &&
            newUniformType != UniformType::NoType;
      }

      if (uniqueValidUniform) {
        Uniform newUniform;
        newUniform.shaderName = newUniformShaderName;
        newUniform.name = newUniformName;
        newUniform.type = newUniformType;

        switch (newUniformType) {
        case UniformType::Int:
          newUniform.value = 0;
          break;
        case UniformType::Float:
          newUniform.value = 0.0f;
          break;
        case UniformType::Vec3:
          newUniform.value = glm::vec3(0.0f);
          break;
        case UniformType::Vec4:
          newUniform.value = glm::vec4(0.0f);
          break;
        default:
          std::cout << "invalid new uniform type, making it an int"
                    << std::endl;
          newUniform.type = UniformType::Int;
          newUniform.value = 0;
          break;
        }

        uniforms[newUniformShaderName][newUniformName] = newUniform;
        newUniformShaderName = "";
        newUniformName = "";
      } else {
        std::cout << "bad new uniform input" << std::endl;
      }
    }
  }

  void drawAddObjectMenu() {
    ImGui::Text("Add Object");
    drawTextInput(&newObjectName, "Object Name");

    // Build a list of shader names
    std::vector<const char *> shaderChoices;
    shaderChoices.reserve(shaders.size() + 1);
    shaderChoices.push_back("");
    for (auto &[name, shader] : shaders) {
      shaderChoices.push_back(name.c_str());
    }
    // Keep track of the selected shader index
    static int shaderChoice = 0;
    if (newObjectName == "")
      shaderChoice = 0;
    // Display combo box
    if (ImGui::Combo("Shader", &shaderChoice, shaderChoices.data(),
                     (int)shaderChoices.size())) {
      // When selection changes, update newObjectName
      newObjectName = shaderChoices[shaderChoice];
    }
    // Dropdown menu from enum
    const char *typeNames[6] = {"None", "Int",  "Float",
                                "Vec3", "Vec4", "Reference"};
    int choice = static_cast<int>(newUniformType);
    if (ImGui::Combo("Uniform Type", &choice, typeNames,
                     IM_ARRAYSIZE(typeNames))) {
      newUniformType = static_cast<UniformType>(choice);
    }

    if (ImGui::Button("Add Uniform", ImVec2(100, 20))) {
      bool uniqueValidUniform = false;
      bool validShaderName = shaders.count(newObjectName) >= 1;
      if (validShaderName) {
        uniqueValidUniform =
            uniforms[newObjectName].count(newUniformName) <= 0 &&
            newUniformType != UniformType::NoType;
      }

      if (uniqueValidUniform) {
        Uniform newUniform;
        newUniform.shaderName = newObjectName;
        newUniform.name = newUniformName;
        newUniform.type = newUniformType;

        switch (newUniformType) {
        case UniformType::Int:
          newUniform.value = 0;
          break;
        case UniformType::Float:
          newUniform.value = 0.0f;
          break;
        case UniformType::Vec3:
          newUniform.value = glm::vec3(0.0f);
          break;
        case UniformType::Vec4:
          newUniform.value = glm::vec4(0.0f);
          break;
        default:
          std::cout << "invalid new uniform type, making it an int"
                    << std::endl;
          newUniform.type = UniformType::Int;
          newUniform.value = 0;
          break;
        }

        uniforms[newObjectName][newUniformName] = newUniform;
        newObjectName = "";
        newUniformName = "";
      } else {
        std::cout << "bad new uniform input" << std::endl;
      }
    }
  }

  vector<std::string> uniformNamesToDelete;
  void drawUniformEditors() {
    drawAddUniformMenu();
    int imGuiID = 0;
    for (auto &[shaderName, shader] : shaders) {
      ImGui::Text("%s", ("Shader:" + shaderName).c_str());
      for (auto &[uniformName, uniform] : uniforms[shaderName]) {
        ImGui::PushID(imGuiID);
        drawUniformInput(shaderName, uniformName);
        ImGui::PopID();
        imGuiID++;
      }
      for(std::string s : uniformNamesToDelete)
        uniforms[shaderName].erase(s);
      uniformNamesToDelete.clear();
    }
  }

  void drawUniformInput(std::string shaderName, std::string uniformName) {
    Uniform *uniform = &(uniforms[shaderName][uniformName]);
    ImGui::Text("%s", uniformName.c_str());
    switch (uniform->type) {
    case UniformType::Int: {
      int intValue = std::get<int>(uniform->value);
      ImGui::InputInt("value", &intValue);
      uniform->value = intValue;
      break;
    }
    case UniformType::Float: {
      float floatValue = std::get<float>(uniform->value);
      ImGui::InputFloat("value", &floatValue);
      uniform->value = floatValue;
      break;
    }
    case UniformType::Vec3: {
      glm::vec3 v3Val = std::get<glm::vec3>(uniform->value);
      ImGui::InputFloat("x", &(v3Val.x));
      ImGui::InputFloat("y", &(v3Val.y));
      ImGui::InputFloat("z", &(v3Val.z));
      uniform->value = v3Val;
      break;
    }
    case UniformType::Vec4: {
      glm::vec4 v4Val = std::get<glm::vec4>(uniform->value);
      ImGui::InputFloat("x", &(v4Val.x));
      ImGui::InputFloat("y", &(v4Val.y));
      ImGui::InputFloat("z", &(v4Val.z));
      ImGui::InputFloat("w", &(v4Val.w));
      uniform->value = v4Val;
      break;
    }
    case UniformType::UniformRef: {
      // Later, I'd like to defer this so you don't need to have all your
      // uniform refs at the end of the file
      std::cout << "no refs yet supported" << std::endl;
    }
    }

    if (ImGui::Button("Delete Uniform", ImVec2(100, 20))) {
      uniformNamesToDelete.push_back(uniformName);
    }
  }
};
