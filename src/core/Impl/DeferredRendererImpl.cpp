//
// Created by Admin on 21/02/2025.
//

#include "DeferredRendererImpl.h"

#include "../_deps/LTCTex.h"

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

  lightingBuffer_tex = new gl::Texture2D;
  lightingBuffer_tex->SetWrapFilter(
      gl::WrapMode::ClampToEdge, gl::WrapMode::ClampToEdge,
      gl::MinFilter::Nearest, gl::MagFilter::Nearest);

  LTC_tsfm.SetImage(0, gl::PixelDataInternalFormat::Rgba32F, LTC::TexSize,
                    LTC::TexSize, gl::PixelDataFormat::Rgba,
                    gl::PixelDataType::Float, LTC::data1);
  LTC_nf0s.SetImage(0, gl::PixelDataInternalFormat::Rgba32F, LTC::TexSize,
                    LTC::TexSize, gl::PixelDataFormat::Rgba,
                    gl::PixelDataType::Float, LTC::data2);

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

  string p3t2_vs_path = "../data/shaders/p3t2.vs";
  string p3t2n3t3_vs_path = "../data/shaders/p3t2n3t3.vs";
  string p2t2_vs_path = "../data/shaders/p2t2.vs";
  string env_vs_path = "../data/shaders/env.vs";
  string stdBRDF_fs_path = "../data/shaders/stdBRDF.fs";
  string light_fs_path = "../data/shaders/light.fs";
  string img_fs_path = "../data/shaders/img.fs";
  string deferred_lighting_fs_path = "../data/shaders/deferred_lighting.fs";
  string env_fs_path = "../data/shaders/env.fs";
  string postprocess_fs_path = "../data/shaders/postprocess.fs";

  auto p3t2_vs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      p3t2_vs_path, gl::ShaderType::VertexShader, p3t2_vs_path);
  auto p3t2n3t3_vs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      p3t2n3t3_vs_path, gl::ShaderType::VertexShader, p3t2n3t3_vs_path);
  auto p2t2_vs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      p2t2_vs_path, gl::ShaderType::VertexShader, p2t2_vs_path);
  auto env_vs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      env_vs_path, gl::ShaderType::VertexShader, env_vs_path);
  auto stdBRDF_fs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      stdBRDF_fs_path, gl::ShaderType::FragmentShader, stdBRDF_fs_path);
  auto light_fs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      light_fs_path, gl::ShaderType::FragmentShader, light_fs_path);
  auto img_fs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      img_fs_path, gl::ShaderType::FragmentShader, img_fs_path);
  auto deferred_lighting_fs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      deferred_lighting_fs_path, gl::ShaderType::FragmentShader,
      deferred_lighting_fs_path);
  auto env_fs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      env_fs_path, gl::ShaderType::FragmentShader, env_fs_path);
  auto postprocess_fs = RsrcMngr<gl::Shader>::Instance().GetOrCreate(
      postprocess_fs_path, gl::ShaderType::FragmentShader, postprocess_fs_path);

  stdBRDFProgram = new gl::Program(p3t2n3t3_vs, stdBRDF_fs);
  lightProgram = new gl::Program(p3t2_vs, light_fs);
  screenProgram = new gl::Program(p2t2_vs, img_fs);
  deferred_lightingProgram = new gl::Program(p2t2_vs, deferred_lighting_fs);
  envProgram = new gl::Program(env_vs, env_fs);
  postprocessProgram = new gl::Program(p2t2_vs, postprocess_fs);

  stdBRDFProgram->SetTex("albedo_texture", 0);
  stdBRDFProgram->SetTex("roughness_texture", 1);
  stdBRDFProgram->SetTex("metalness_texture", 2);
  stdBRDFProgram->SetTex("normalmap", 3);

  lightProgram->SetTex("radiance_texture", 0);

  screenProgram->SetTex("img", 0);
  envProgram->SetTex("EnvLight_texture", 0);

  deferred_lightingProgram->SetTex("gbuffer0", 0);
  deferred_lightingProgram->SetTex("gbuffer1", 1);
  deferred_lightingProgram->SetTex("gbuffer2", 2);
  deferred_lightingProgram->SetTex("gbuffer3", 3);
  deferred_lightingProgram->SetTex("LTC_tsfm", 4);
  deferred_lightingProgram->SetTex("LTC_nf0s", 5);

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

  auto cubeMesh = TriMesh(TriMesh::Type::Cube);
  cube =
      new gl::Mesh(gl::BasicPrimitiveType::Triangles, cubeMesh.indices->size(),
                   cubeMesh.positions->size(), cubeMesh.indices->data()->data(),
                   {make_tuple(cubeMesh.positions->data()->data(), 3),
                    make_tuple(cubeMesh.texcoords->data()->data(), 2),
                    make_tuple(cubeMesh.normals->data()->data(), 3),
                    make_tuple(cubeMesh.tangents->data()->data(), 3)});

  auto squareMesh = TriMesh(TriMesh::Type::Square);
  square = new gl::Mesh(
      gl::BasicPrimitiveType::Triangles, squareMesh.indices->size(),
      squareMesh.positions->size(), squareMesh.indices->data()->data(),
      {make_tuple(squareMesh.positions->data()->data(), 3),
       make_tuple(squareMesh.texcoords->data()->data(), 2),
       make_tuple(squareMesh.normals->data()->data(), 3),
       make_tuple(squareMesh.tangents->data()->data(), 3)});
}

DeferredRenderer::Impl::~Impl() {
  for (auto tex : gtexs)
    delete tex;
  delete lightingBuffer_tex;

  delete depth;

  delete stdBRDFProgram;
  delete deferred_lightingProgram;
  delete envProgram;
  delete postprocessProgram;
  delete screenProgram;

  for (const auto& [primitive, rec] : primitive2mesh)
    delete rec;

  delete screen;
  delete sphere;
  delete cube;
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
  } else if (vtable_is<Square>(primitive)) {
    return square;
  } else
    return nullptr;
}

gl::Texture2D* DeferredRenderer::Impl::GetGLTex2D(const Texture2D* tex,
                                                  DefaultTex default_tex,
                                                  TexPrecision precision) {
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
  gl::PixelDataInternalFormat c2if[4];
  switch (precision) {
    case Ubpa::DeferredRenderer::Impl::TexPrecision::F16:
      c2if[0] = gl::PixelDataInternalFormat::R16F;
      c2if[1] = gl::PixelDataInternalFormat::Rg16F;
      c2if[2] = gl::PixelDataInternalFormat::Rgb16F;
      c2if[3] = gl::PixelDataInternalFormat::Rgba16F;
      break;
    case Ubpa::DeferredRenderer::Impl::TexPrecision::F32:
      c2if[0] = gl::PixelDataInternalFormat::R32F;
      c2if[1] = gl::PixelDataInternalFormat::Rg32F;
      c2if[2] = gl::PixelDataInternalFormat::Rgb32F;
      c2if[3] = gl::PixelDataInternalFormat::Rgba32F;
      break;
    case Ubpa::DeferredRenderer::Impl::TexPrecision::Byte8:
    default:
      c2if[0] = gl::PixelDataInternalFormat::Red;
      c2if[1] = gl::PixelDataInternalFormat::Rg;
      c2if[2] = gl::PixelDataInternalFormat::Rgb;
      c2if[3] = gl::PixelDataInternalFormat::Rgba;
      break;
  }
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
  lightingBuffer_tex->SetImage(0, gl::PixelDataInternalFormat::Rgb32F, width,
                               height, gl::PixelDataFormat::Rgb,
                               gl::PixelDataType::Float, nullptr);

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

  lightingBuffer.Attach(gl::FramebufferAttachment::ColorAttachment0,
                        lightingBuffer_tex);
  lightingBuffer.Attach(gl::FramebufferAttachment::DepthAttachment, depth);

  this->width = width;
  this->height = height;
}

void DeferredRenderer::Impl::RenderImpl(Scene* scene, SObj* camObj,
                                        size_t width, size_t height) {
  ResizeBuffer(width, height);

  // camera
  auto camera = camObj->Get<Cmpt::Camera>();
  assert(camera != nullptr);
  auto cam_pos = camObj->Get<Cmpt::L2W>()->WorldPos();
  auto cam_front = (-camObj->Get<Cmpt::L2W>()->FrontInWorld()).normalize();
  stdBRDFProgram->SetMatf4("view",
                           transformf::look_at(cam_pos, cam_pos + cam_front));
  stdBRDFProgram->SetMatf4(
      "projection",
      transformf::perspective(camera->fov, camera->ar, 0.1f, 100.f));
  envProgram->SetMatf4("view",
                       transformf::look_at(cam_pos, cam_pos + cam_front));
  envProgram->SetMatf4("projection", transformf::perspective(
                                         camera->fov, camera->ar, 0.1f, 100.f));
  lightProgram->SetMatf4("view",
                         transformf::look_at(cam_pos, cam_pos + cam_front));
  lightProgram->SetMatf4(
      "projection",
      transformf::perspective(camera->fov, camera->ar, 0.1f, 100.f));
  deferred_lightingProgram->SetVecf3("camera_pos", cam_pos);

  // set point/rect lights and env light
  size_t pointLightNum = 0;
  size_t rectLightNum = 0;
  const EnvLight* envLight = nullptr;
  scene->Each([this, &envLight, &pointLightNum, &rectLightNum](
                  Cmpt::Light* light, Cmpt::SObjPtr* sobjptr, Cmpt::L2W* l2w) {
    if (vtable_is<PointLight>(light->light.get())) {
      auto pointLight = static_cast<const PointLight*>(light->light.get());
      string obj = string("pointlights[") + to_string(pointLightNum++) + "]";
      deferred_lightingProgram->SetVecf3((obj + ".position").c_str(),
                                         l2w->WorldPos());
      deferred_lightingProgram->SetVecf3(
          (obj + ".radiance").c_str(),
          pointLight->intensity * pointLight->color);
    } else if (!envLight && vtable_is<EnvLight>(light->light.get())) {
      envLight = static_cast<const EnvLight*>(light->light.get());
      envProgram->SetVecf3("EnvLight_color", envLight->color);
      envProgram->SetFloat("EnvLight_intensity", envLight->intensity);
    } else if (vtable_is<AreaLight>(light->light.get())) {
      auto areaLight = static_cast<const AreaLight*>(light->light.get());
      auto geo = sobjptr->sobj->Get<Cmpt::Geometry>();
      if (geo) {
        if (vtable_is<Square>(geo->primitive.get())) {
          string obj = string("rectlights[") + to_string(rectLightNum++) + "]";
          auto pos = l2w->WorldPos();
          auto dir = l2w->UpInWorld().normalize();
          auto right = l2w->RightInWorld().normalize();
          auto scale = l2w->value->decompose_scale();
          auto width = 2.f * scale[0];
          auto height = 2.f * scale[2];
          deferred_lightingProgram->SetVecf3((obj + ".position").c_str(), pos);
          deferred_lightingProgram->SetVecf3((obj + ".dir").c_str(), dir);
          deferred_lightingProgram->SetVecf3((obj + ".right").c_str(), right);
          deferred_lightingProgram->SetFloat((obj + ".width").c_str(), width);
          deferred_lightingProgram->SetFloat((obj + ".height").c_str(), height);
          deferred_lightingProgram->SetVecf3(
              (obj + ".radiance").c_str(),
              areaLight->color * areaLight->intensity);
        }
      }
    }
  });
  if (!envLight)
    envProgram->SetFloat("EnvLight_intensity", 0.f);
  deferred_lightingProgram->SetUInt("pointlight_num",
                                    static_cast<GLuint>(pointLightNum));
  deferred_lightingProgram->SetUInt("rectlight_num",
                                    static_cast<GLuint>(rectLightNum));

  // [pass 1] GBuffer pass
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
      stdBRDFProgram->SetFloat("metalness_factor", brdf->metalness_factor);
      stdBRDFProgram->SetVecf3("albedo_factor", brdf->albedo_factor);
      stdBRDFProgram->SetFloat("roughness_factor", brdf->roughness_factor);

      stdBRDFProgram->Active(0, GetGLTex2D(brdf->albedo_texture));
      stdBRDFProgram->Active(1, GetGLTex2D(brdf->roughness_texture));
      stdBRDFProgram->Active(2, GetGLTex2D(brdf->metalness_texture));
      stdBRDFProgram->Active(3,
                             GetGLTex2D(brdf->normal_map, DefaultTex::Normal));
    }
    stdBRDFProgram->SetMatf4("model", l2w->value);
    mesh->Draw(*stdBRDFProgram);
  });
  scene->Each([this](Cmpt::Geometry* geo, Cmpt::Light* light, Cmpt::L2W* l2w) {
    Primitive* primitive = geo->primitive;
    auto mesh = GetPrimitiveMesh(primitive);
    if (!mesh)
      return;
    if (vtable_is<AreaLight>(light->light.get())) {
      auto arealight = static_cast<const AreaLight*>(light->light.get());
      lightProgram->SetVecf3("radiance_factor",
                             arealight->color * arealight->intensity);
      lightProgram->Active(0, GetGLTex2D(arealight->texture));
    }
    lightProgram->SetMatf4("model", l2w->value);
    mesh->Draw(*lightProgram);
  });

  // [pass 2] lighting pass
  lightingBuffer.Bind();
  gl::Disable(gl::Capability::DepthTest);

  deferred_lightingProgram->Active(
      0, gb.GetTex2D(gl::FramebufferAttachment::ColorAttachment0));
  deferred_lightingProgram->Active(
      1, gb.GetTex2D(gl::FramebufferAttachment::ColorAttachment1));
  deferred_lightingProgram->Active(
      2, gb.GetTex2D(gl::FramebufferAttachment::ColorAttachment2));
  deferred_lightingProgram->Active(
      3, gb.GetTex2D(gl::FramebufferAttachment::ColorAttachment3));
  deferred_lightingProgram->Active(4, &LTC_tsfm);
  deferred_lightingProgram->Active(5, &LTC_nf0s);

  screen->Draw(*deferred_lightingProgram);

  gl::Enable(gl::Capability::DepthTest);
  gl::DepthFunc(gl::CompareFunc::Lequal);

  if (envLight)
    envProgram->Active(
        0, GetGLTex2D(envLight->texture, DefaultTex::White, TexPrecision::F32));

  cube->Draw(*envProgram);
  gl::DepthFunc(gl::CompareFunc::Less);

  // [pass 3] postprocess pass
  gl::FrameBuffer::BindReset();
  gl::Disable(gl::Capability::DepthTest);

  postprocessProgram->Active(
      0, lightingBuffer.GetTex2D(gl::FramebufferAttachment::ColorAttachment0));

  screen->Draw(*postprocessProgram);
}
