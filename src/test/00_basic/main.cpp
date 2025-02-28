//
// Created by Admin on 26/02/2025.
//

#include <MyGL/MyGL.h>

#include <GLFW/glfw3.h>

#include <MyRTR/DeferredRenderer.h>

#include <MyScene/MyScene.h>

#include <iostream>

using namespace My;
using namespace std;

constexpr size_t SCR_WIDTH = 1280;
constexpr size_t SCR_HEIGHT = 720;

//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

struct Rotater : Component {
  float speed{1.f};

  static void OnRegist() {
    Reflection<Rotater>::Instance().Regist(&Rotater::speed,
                                           NAMEOF(Rotater::speed).c_str());
  }

  void OnUpdate(Cmpt::Rotation* rot) const {
    rot->value = quatf{vecf3{1.f}, speed * to_radian(1.f)} * rot->value;
  }
};

int main() {
  sizeof(Cmpt::Rotation);

  SceneReflectionInit();

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
      SCR_WIDTH, SCR_HEIGHT, "Ubpa@2020 : test - 01 - defer", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  //   glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  //   glfwSetCursorPosCallback(window, mouse_callback);
  //   glfwSetScrollCallback(window, scroll_callback);

  // tell GLFW to capture our mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  Scene scene("scene");

  auto [sobj0, camera] = scene.CreateSObj<Cmpt::Camera>("sobj0");
  auto [sobj1, geo1, mat1] =
      scene.CreateSObj<Cmpt::Geometry, Cmpt::Material>("sobj1");
  auto [sobj2, geo2, mat2] =
      scene.CreateSObj<Cmpt::Geometry, Cmpt::Material>("sobj2");
  auto [sobj3, geo3, mat3, r3] =
      scene.CreateSObj<Cmpt::Geometry, Cmpt::Material, Rotater>("sobj3");
  auto [sobj4, light4] = scene.CreateSObj<Cmpt::Light>("sobj4");
  auto [sobj5, env] = scene.CreateSObj<Cmpt::Light>("sobj5");
  auto [sobj6, light6, geo6, r6] =
      scene.CreateSObj<Cmpt::Light, Cmpt::Geometry, Rotater>("sobj6");

  string albedo_path = "../data/textures/rusted_iron/albedo.png";
  string roughness_path = "../data/textures/rusted_iron/roughness.png";
  string metalness_path = "../data/textures/rusted_iron/metallic.png";
  string normal_path = "../data/textures/rusted_iron/normal.png";
  string env_path = "../data/textures/newport_loft.hdr";
  auto albedo_texture = new Texture2D(albedo_path);
  auto roughness_texture = new Texture2D(roughness_path);
  auto metalness_texture = new Texture2D(metalness_path);
  auto normals_texture = new Texture2D(normal_path);
  auto env_texture = new Texture2D(env_path);

  geo1->SetPrimitive(new Sphere);
  geo2->SetPrimitive(new Square);
  geo3->SetPrimitive(new TriMesh(TriMesh::Type::Cube));
  auto brdf1 = new stdBRDF;
  auto brdf2 = new stdBRDF;
  auto brdf3 = new stdBRDF;
  brdf1->albedo_texture = albedo_texture;
  brdf1->roughness_texture = roughness_texture;
  brdf1->metalness_texture = metalness_texture;
  brdf1->normal_map = normals_texture;
  brdf2->albedo_texture = albedo_texture;
  brdf2->roughness_texture = roughness_texture;
  brdf2->metalness_texture = metalness_texture;
  brdf2->normal_map = normals_texture;
  brdf3->albedo_texture = albedo_texture;
  brdf3->roughness_texture = roughness_texture;
  brdf3->metalness_texture = metalness_texture;
  brdf3->normal_map = normals_texture;
  mat1->SetMaterial(brdf1);
  mat2->SetMaterial(brdf2);
  mat3->SetMaterial(brdf3);

  sobj0->Get<Cmpt::Position>()->value = {0, 0, 8};
  sobj1->Get<Cmpt::Position>()->value = {-4, 0, 0};
  sobj1->Get<Cmpt::Scale>()->value = 2.f;
  sobj2->Get<Cmpt::Rotation>()->value = {vecf3{1, 0, 0}, to_radian(45.f)};
  sobj3->Get<Cmpt::Position>()->value = {4, 0, 0};
  sobj3->Get<Cmpt::Scale>()->value = {1, 2, 1};
  sobj3->Get<Cmpt::Rotation>()->value = {vecf3{1, 2, 1}.normalize(),
                                         to_radian(45.f)};
  camera->fov = to_radian(60.f);
  camera->ar = SCR_WIDTH / static_cast<float>(SCR_HEIGHT);

  light4->light = new PointLight{100.f, {0.9f, 0.9f, 1.f}};
  sobj4->Get<Cmpt::Position>()->value = {0, 4, 0};

  light6->light = new AreaLight{100.f, {1, 0, 1}};
  geo6->SetPrimitive(new Square);
  sobj6->Get<Cmpt::Position>()->value = {0, 3, 0};
  sobj6->Get<Cmpt::Rotation>()->value =
      quatf{vecf3{1, 0, 0}, to_radian(180.f)} *
      sobj6->Get<Cmpt::Rotation>()->value;

  env->SetLight(new EnvLight(1.f, rgbf{1.f}, env_texture));

  SerializerJSON serializer;
  auto rst = serializer.Serialize(&scene);
  cout << rst << endl;

  DeferredRenderer rtr;

  scene.Start();

  while (!glfwWindowShouldClose(window)) {
    // input
    // -----
    processInput(window);

    scene.Update();
    rtr.Render(&scene, sobj0, SCR_WIDTH, SCR_HEIGHT);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  scene.Stop();

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