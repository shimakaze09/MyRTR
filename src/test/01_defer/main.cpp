//
// Created by Admin on 21/02/2025.
//

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <MyRTR/DeferredRenderer.h>

#include <MyGL/MyGL>

#include <MyScene/core/core>

#include <iostream>

using namespace My;
using namespace std;

constexpr size_t SCR_WIDTH = 1280;
constexpr size_t SCR_HEIGHT = 720;

//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main() {

  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(
      GLFW_OPENGL_FORWARD_COMPAT,
      GL_TRUE);  // uncomment this statement to fix compilation on OS X
#endif

  // glfw window creation
  // --------------------
  GLFWwindow* window = glfwCreateWindow(
      SCR_WIDTH, SCR_HEIGHT, "My@2025 : test - 01 - defer", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  /*glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);*/

  // tell GLFW to capture our mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  Scene scene("scene");

  auto [sobj0, tsfm0, camera] =
      scene.CreateSObj<Cmpt::Transform, Cmpt::Camera>("sobj0");
  auto [sobj1, tsfm1, geo1, mat1] =
      scene.CreateSObj<Cmpt::Transform, Cmpt::Geometry, Cmpt::Material>(
          "sobj1");
  auto [sobj2, tsfm2, geo2, mat2] =
      scene.CreateSObj<Cmpt::Transform, Cmpt::Geometry, Cmpt::Material>(
          "sobj2");
  auto [sobj3, tsfm3, geo3, mat3] =
      scene.CreateSObj<Cmpt::Transform, Cmpt::Geometry, Cmpt::Material>(
          "sobj3");

  geo1->SetPrimitive(new Sphere);
  geo2->SetPrimitive(new Square);
  geo3->SetPrimitive(new TriMesh(TriMesh::Type::Cube));
  auto brdf1 = new stdBRDF;
  auto brdf2 = new stdBRDF;
  auto brdf3 = new stdBRDF;
  string albedo_path = "../data/textures/rusted_iron/albedo.png";
  string roughness_path = "../data/textures/rusted_iron/roughness.png";
  string metalness_path = "../data/textures/rusted_iron/metallic.png";
  brdf1->albedo_texture =
      ResourceMngr<Image>::Instance().GetOrCreate(albedo_path, albedo_path);
  brdf1->roughness_texture = ResourceMngr<Image>::Instance().GetOrCreate(
      roughness_path, roughness_path);
  brdf1->metalness_texture = ResourceMngr<Image>::Instance().GetOrCreate(
      metalness_path, metalness_path);
  brdf2->albedo_texture =
      ResourceMngr<Image>::Instance().GetOrCreate(albedo_path, albedo_path);
  brdf2->roughness_texture = ResourceMngr<Image>::Instance().GetOrCreate(
      roughness_path, roughness_path);
  brdf2->metalness_texture = ResourceMngr<Image>::Instance().GetOrCreate(
      metalness_path, metalness_path);
  brdf3->albedo_texture =
      ResourceMngr<Image>::Instance().GetOrCreate(albedo_path, albedo_path);
  brdf3->roughness_texture = ResourceMngr<Image>::Instance().GetOrCreate(
      roughness_path, roughness_path);
  brdf3->metalness_texture = ResourceMngr<Image>::Instance().GetOrCreate(
      metalness_path, metalness_path);
  mat1->SetMaterial(brdf1);
  mat2->SetMaterial(brdf2);
  mat3->SetMaterial(brdf3);

  tsfm0->SetPosition({0, 0, 8});
  tsfm1->SetPosition({-4, 0, 0});
  tsfm2->SetRotation({vecf3{1, 0, 0}, to_radian(45.f)});
  tsfm3->SetPosition({4, 0, 0});
  tsfm3->SetScale({1, 2, 1});
  tsfm3->SetRotation({vecf3{1, 2, 1}.normalize(), to_radian(45.f)});
  camera->Init(to_radian(60.f), SCR_WIDTH / static_cast<float>(SCR_HEIGHT));

  DeferredRenderer rtr;

  while (!glfwWindowShouldClose(window)) {
    // input
    // -----
    processInput(window);

    rtr.Render(&scene, sobj0, SCR_WIDTH, SCR_HEIGHT);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  //   if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
  //     camera.ProcessKeyboard(FORWARD, deltaTime);
  //   if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
  //     camera.ProcessKeyboard(BACKWARD, deltaTime);
  //   if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
  //     camera.ProcessKeyboard(LEFT, deltaTime);
  //   if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
  //     camera.ProcessKeyboard(RIGHT, deltaTime);
}
