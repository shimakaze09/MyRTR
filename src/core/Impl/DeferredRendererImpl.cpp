//
// Created by Admin on 21/02/2025.
//

#include "DeferredRendererImpl.h"

#include <MyScene/core.h>

#include <MyBL/RsrcMngr.h>
#include <MyDP/Basic/vtable.h>

using namespace My;
using namespace std;

DeferredRenderer::Impl::Impl() : depth{new gl::RenderBuffer} {
  for (auto& tex : gtexs) {
    tex = new gl::Texture2D;
    tex->SetWrapFilter(gl::WrapMode::ClampToEdge, gl::WrapMode::ClampToEdge,
                       gl::MinFilter::Nearest, gl::MagFilter::Nearest);
  }

  float white_data[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  default_white.SetImage(0, gl::PixelDataInternalFormat::Rgba, 2, 2,
                         gl::PixelDataFormat::Rgba, gl::PixelDataType::Float,
                         white_data);
  default_white.SetWrapFilter(gl::WrapMode::Repeat, gl::WrapMode::Repeat,
                              gl::MinFilter::Nearest, gl::MagFilter::Nearest);
  float normal_data[12] = {0.5, 0.5, 1, 0.5, 0.5, 1, 0.5, 0.5, 1, 0.5, 0.5, 1};
  default_normal.SetImage(0, gl::PixelDataInternalFormat::Rgb, 2, 2,
                          gl::PixelDataFormat::Rgb, gl::PixelDataType::Float,
                          normal_data);
  default_normal.SetWrapFilter(gl::WrapMode::Repeat, gl::WrapMode::Repeat,
                               gl::MinFilter::Nearest, gl::MagFilter::Nearest);

  string p3t2n3t3_vs_path = "../data/shaders/p3t2n3t3.vs";
  string p2t2_vs_path = "../data/shaders/p2t2.vs";
  string gbuffer_fs_path = "../data/shaders/gbuffer.fs";
  string img_fs_path = "../data/shaders/img.fs";
  string deferredlight_fs_path = "../data/shaders/deferredlight.fs";

  auto p3t2n3t3_vs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      p3t2n3t3_vs_path, gl::ShaderType::VertexShader, p3t2n3t3_vs_path);
  auto gbuffer_fs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      gbuffer_fs_path, gl::ShaderType::FragmentShader, gbuffer_fs_path);
  auto p2t2_vs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      p2t2_vs_path, gl::ShaderType::VertexShader, p2t2_vs_path);
  auto img_fs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      img_fs_path, gl::ShaderType::FragmentShader, img_fs_path);
  auto deferredlight_fs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      deferredlight_fs_path, gl::ShaderType::FragmentShader,
      deferredlight_fs_path);

  gProgram = new gl::Program(p3t2n3t3_vs, gbuffer_fs);
  screenProgram = new gl::Program(p2t2_vs, img_fs);
  deferredlightProgram = new gl::Program(p2t2_vs, deferredlight_fs);

  gProgram->SetTex("albedo_texture", 0);
  gProgram->SetTex("roughness_texture", 1);
  gProgram->SetTex("metalness_texture", 2);
  gProgram->SetTex("normalmap", 3);

  screenProgram->SetTex("texture0", 0);
  deferredlightProgram->SetTex("gbuffer0", 0);
  deferredlightProgram->SetTex("gbuffer1", 1);
  deferredlightProgram->SetTex("gbuffer2", 2);
  deferredlightProgram->SetTex("gbuffer3", 3);

  // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
  float quad_vertices[] = {// positions   // texCoords
                           -1.0f, 1.0f,  0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                           1.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  1.0f, 1.0f};
  unsigned int quad_indices[] = {2, 3, 1, 0, 1, 3};
  screen = new gl::Mesh(gl::BasicPrimitiveType::Triangles, 2, 4, quad_indices,
                        quad_vertices, {2, 2});

  auto sphereMesh = TriMesh(TriMesh::Type::Sphere);
  sphere = new gl::Mesh(
      gl::BasicPrimitiveType::Triangles, sphereMesh.indices->size(),
      sphereMesh.positions->size(), sphereMesh.indices->data()->data(),
      {make_tuple(sphereMesh.positions->data()->data(), 3),
       make_tuple(sphereMesh.texcoords->data()->data(), 2),
       make_tuple(sphereMesh.normals->data()->data(), 3),
       make_tuple(sphereMesh.tangents->data()->data(), 3)});
}

DeferredRenderer::Impl::~Impl() {
  for (auto tex : gtexs)
    delete tex;

  delete depth;

  delete gProgram;
  delete screenProgram;
  delete deferredlightProgram;

  for (const auto& [primitive, rec] : primitive2mesh)
    delete rec;

  delete screen;
  delete sphere;
}

gl::Mesh* DeferredRenderer::Impl::GetPrimitiveMesh(const Primitive* primitive) {
  if (vtable_is<TriMesh>(primitive)) {
    auto target = primitive2mesh.find(primitive);
    if (target != primitive2mesh.end())
      return target->second;
    auto trimesh = static_cast<const TriMesh*>(primitive);
    auto mesh = new gl::Mesh(
        gl::BasicPrimitiveType::Triangles, trimesh->indices->size(),
        trimesh->positions->size(), trimesh->indices->data()->data(),
        {make_tuple(trimesh->positions->data()->data(), 3),
         make_tuple(trimesh->texcoords->data()->data(), 2),
         make_tuple(trimesh->normals->data()->data(), 3),
         make_tuple(trimesh->tangents->data()->data(), 3)});

    primitive2mesh[primitive] = mesh;
    return mesh;
  } else if (vtable_is<Sphere>(primitive)) {
    return sphere;
  } else
    return nullptr;
}

gl::Texture2D* DeferredRenderer::Impl::GetGLTex2D(const Texture2D* tex,
                                                  DefaultTex default_tex) {
  if (tex == nullptr) {
    switch (default_tex) {
      case Ubpa::DeferredRenderer::Impl::DefaultTex::White:
        return &default_white;
      case Ubpa::DeferredRenderer::Impl::DefaultTex::Normal:
        return &default_normal;
      default:
        return nullptr;
    }
  }

  auto tex2d = RsrcMngr<gl::Texture2D, const Texture2D*>::Instance().Get(tex);
  if (tex2d != nullptr)
    return tex2d;

  auto new_tex = new gl::Texture2D;
  gl::PixelDataFormat c2f[4] = {
      gl::PixelDataFormat::Red, gl::PixelDataFormat::Rg,
      gl::PixelDataFormat::Rgb, gl::PixelDataFormat::Rgba};
  gl::PixelDataInternalFormat c2if[4] = {
      gl::PixelDataInternalFormat::Red, gl::PixelDataInternalFormat::Rg,
      gl::PixelDataInternalFormat::Rgb, gl::PixelDataInternalFormat::Rgba};
  map<Texture2D::WrapMode, gl::WrapMode> wrapmodemap = {
      {Texture2D::WrapMode::Clamp, gl::WrapMode::ClampToEdge},
      {Texture2D::WrapMode::Repeat, gl::WrapMode::Repeat},
      {Texture2D::WrapMode::Mirror, gl::WrapMode::MirroredRepeat}};
  map<Texture2D::SampleMode, gl::MagFilter> samplemodemap = {
      {Texture2D::SampleMode::Nearest, gl::MagFilter::Nearest},
      {Texture2D::SampleMode::Linear, gl::MagFilter::Linear}};
  new_tex->SetImage(0, c2if[tex->img->channel - 1], tex->img->width,
                    tex->img->height, c2f[tex->img->channel - 1],
                    gl::PixelDataType::Float, tex->img->data.get());
  new_tex->SetWrapFilter(
      wrapmodemap[tex->wrap_u], wrapmodemap[tex->wrap_v],
      static_cast<gl::MinFilter>(samplemodemap[tex->sample_mode]),
      samplemodemap[tex->sample_mode]);

  RsrcMngr<gl::Texture2D, const Texture2D*>::Instance().Regist(tex, new_tex);

  return new_tex;
}

void DeferredRenderer::Impl::ResizeBuffer(size_t width, size_t height) {
  if (this->width == width && this->height == height)
    return;

  for (auto tex : gtexs) {
    tex->SetImage(0, gl::PixelDataInternalFormat::Rgba32F, width, height,
                  gl::PixelDataFormat::Rgba, gl::PixelDataType::Float, nullptr);
  }

  depth->SetStorage(gl::FramebufferInternalFormat::DepthComponent, width,
                    height);

  gb.Attach(gl::FramebufferAttachment::ColorAttachment0, gtexs[0]);
  gb.Attach(gl::FramebufferAttachment::ColorAttachment1, gtexs[1]);
  gb.Attach(gl::FramebufferAttachment::ColorAttachment2, gtexs[2]);
  gb.Attach(gl::FramebufferAttachment::ColorAttachment3, gtexs[3]);
  gb.DrawBuffers(
      {gl::ColorBuffer::ColorAttachment0, gl::ColorBuffer::ColorAttachment1,
       gl::ColorBuffer::ColorAttachment2, gl::ColorBuffer::ColorAttachment3});

  gb.Attach(gl::FramebufferAttachment::DepthAttachment, depth);

  this->width = width;
  this->height = height;
}

void DeferredRenderer::Impl::RenderImpl(Scene* scene, SObj* camObj,
                                        size_t width, size_t height) {
  ResizeBuffer(width, height);

  // set pointlights
  size_t pointLightNum = 0;
  scene->Each([this, &pointLightNum](Cmpt::Light* light, Cmpt::Position* pos) {
    if (vtable_is<PointLight>(light->light.get())) {
      auto pointLight = static_cast<const PointLight*>(light->light.get());
      string obj = string("pointlights[") + to_string(pointLightNum++) + "]";
      deferredlightProgram->SetVecf3((obj + ".position").c_str(), pos->value);
      deferredlightProgram->SetVecf3((obj + ".radiance").c_str(),
                                     pointLight->intensity * pointLight->color);
    }
  });
  deferredlightProgram->SetUInt("pointlight_num",
                                static_cast<GLuint>(pointLightNum));

  // camera
  auto camera = camObj->Get<Cmpt::Camera>();
  assert(camera != nullptr);
  auto cam_l2w = camObj->Get<Cmpt::L2W>()->value;
  auto cam_pos = cam_l2w * pointf3{0.f};
  auto cam_front = cam_l2w * vecf3{0, 0, -1};
  gProgram->SetMatf4("view", transformf::look_at(cam_pos, cam_pos + cam_front));
  gProgram->SetMatf4("projection", transformf::perspective(
                                       camera->fov, camera->ar, 0.1f, 100.f));
  deferredlightProgram->SetVecf3("camera_pos", cam_pos);

  gb.Bind();
  gl::Enable(gl::Capability::DepthTest);
  gl::ClearColor({0.f, 0.f, 0.f, 0.f});
  gl::Clear(gl::BufferSelectBit::ColorBufferBit |
            gl::BufferSelectBit::DepthBufferBit |
            gl::BufferSelectBit::StencilBufferBit);
  scene->Each([this](Cmpt::Geometry* geo, Cmpt::Material* mat, Cmpt::L2W* l2w) {
    Primitive* primitive = geo->primitive;
    auto mesh = GetPrimitiveMesh(primitive);
    if (!mesh)
      return;
    if (vtable_is<stdBRDF>(mat->material.get())) {
      auto brdf = static_cast<const stdBRDF*>(mat->material.get());
      gProgram->SetFloat("metalness_factor", brdf->metalness_factor);
      gProgram->SetVecf3("albedo_factor", brdf->albedo_factor);
      gProgram->SetFloat("roughness_factor", brdf->roughness_factor);

      gProgram->Active(0, GetGLTex2D(brdf->albedo_texture));
      gProgram->Active(1, GetGLTex2D(brdf->roughness_texture));
      gProgram->Active(2, GetGLTex2D(brdf->metalness_texture));
      gProgram->Active(3, GetGLTex2D(brdf->normal_map, DefaultTex::Normal));
    }
    gProgram->SetMatf4("model", l2w->value);
    mesh->Draw(*gProgram);
  });

  gl::FrameBuffer::BindReset();
  gl::Disable(gl::Capability::DepthTest);

  deferredlightProgram->Active(
      0, gb.GetTex2D(gl::FramebufferAttachment::ColorAttachment0));
  deferredlightProgram->Active(
      1, gb.GetTex2D(gl::FramebufferAttachment::ColorAttachment1));
  deferredlightProgram->Active(
      2, gb.GetTex2D(gl::FramebufferAttachment::ColorAttachment2));
  deferredlightProgram->Active(
      3, gb.GetTex2D(gl::FramebufferAttachment::ColorAttachment3));

  deferredlightProgram->SetVecf3("pointlight_pos", {0, 3, 0});
  deferredlightProgram->SetVecf3("pointlight_radiance", {100, 100, 120});

  screen->Draw(*deferredlightProgram);
}
