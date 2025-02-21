//
// Created by Admin on 21/02/2025.
//

#include "DeferredRendererImpl.h"

#include <MyScene/core/core>

#include <MyDP/Basic/vtable.h>

using namespace My;
using namespace std;

DeferredRenderer::Impl::Impl()
    : tex0{new gl::Texture2D}, depth{new gl::RenderBuffer} {
  tex0->SetWrapFilter(gl::WrapMode::ClampToEdge, gl::WrapMode::ClampToEdge,
                      gl::MinFilter::Nearest, gl::MagFilter::Nearest);

  string p3t2n3t3_vs_path = "../data/shaders/p3t2n3t3.vs";
  string p2t2_vs_path = "../data/shaders/p2t2.vs";
  string gbuffer_fs_path = "../data/shaders/gbuffer.fs";
  string img_fs_path = "../data/shaders/img.fs";

  auto p3t2n3t3_vs =
      new gl::Shader(gl::ShaderType::VertexShader, p3t2n3t3_vs_path);
  auto gbuffer_fs =
      new gl::Shader(gl::ShaderType::FragmentShader, gbuffer_fs_path);
  auto p2t2_vs = new gl::Shader(gl::ShaderType::VertexShader, p2t2_vs_path);
  auto img_fs = new gl::Shader(gl::ShaderType::FragmentShader, img_fs_path);

  path2shader[p3t2n3t3_vs_path] = p3t2n3t3_vs;
  path2shader[gbuffer_fs_path] = gbuffer_fs;
  path2shader[p2t2_vs_path] = p2t2_vs;
  path2shader[img_fs_path] = img_fs;

  gProgram = new gl::Program(p3t2n3t3_vs, gbuffer_fs);
  screenProgram = new gl::Program(p2t2_vs, img_fs);

  screenProgram->SetTex("texture0", 0);

  // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
  float quad_vertices[] = {// positions   // texCoords
                           -1.0f, 1.0f,  0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                           1.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  1.0f, 1.0f};
  unsigned int quad_indices[] = {2, 3, 1, 0, 1, 3};
  screen = new PrimitiveResource;
  screen->eb =
      new gl::ElementBuffer(gl::BasicPrimitiveType::Triangles, 2, quad_indices);
  gl::VertexArray::Format quad_format;
  screen->n2vb["pos_uv"] = new gl::VertexBuffer(
      sizeof(quad_vertices), quad_vertices, gl::BufferUsage::StaticDraw);
  quad_format.attrptrs.push_back(screen->n2vb["pos_uv"]->AttrPtr(
      2, gl::DataType::Float, GL_FALSE, 4 * sizeof(GLfloat),
      (const void*)(0 * sizeof(float))));
  quad_format.attrptrs.push_back(screen->n2vb["pos_uv"]->AttrPtr(
      2, gl::DataType::Float, GL_FALSE, 4 * sizeof(GLfloat),
      (const void*)(2 * sizeof(float))));
  quad_format.eb = screen->eb;
  screen->va = new gl::VertexArray{{0, 1}, quad_format};
}

DeferredRenderer::Impl::~Impl() {
  delete tex0;
  delete depth;
  delete gProgram;
  delete screenProgram;
  for (const auto& [path, shader] : path2shader)
    delete shader;
  for (const auto& [primitive, rec] : p2r)
    delete rec;
  delete screen;
}

gl::VertexArray* DeferredRenderer::Impl::GetPrimitiveVAO(
    const Primitive* primitive) {
  auto target = p2r.find(primitive);
  if (target != p2r.end())
    return target->second->va;

  if (vtable_is<TriMesh>(primitive)) {
    auto trimesh = static_cast<const TriMesh*>(primitive);
    auto pos = new gl::VertexBuffer(
        trimesh->positions->size() * sizeof(pointf3),
        trimesh->positions->data()->data(), gl::BufferUsage::DynamicDraw);
    auto uv = new gl::VertexBuffer(trimesh->texcoords->size() * sizeof(pointf2),
                                   trimesh->texcoords->data()->data(),
                                   gl::BufferUsage::DynamicDraw);
    auto normal = new gl::VertexBuffer(
        trimesh->normals->size() * sizeof(normalf),
        trimesh->normals->data()->data(), gl::BufferUsage::DynamicDraw);
    auto tangent = new gl::VertexBuffer(
        trimesh->tangents->size() * sizeof(vecf3),
        trimesh->tangents->data()->data(), gl::BufferUsage::DynamicDraw);
    auto indices = new gl::ElementBuffer(gl::BasicPrimitiveType::Triangles,
                                         trimesh->indices->size(),
                                         trimesh->indices->data()->data());
    gl::VertexArray::Format format;
    format.attrptrs.push_back(
        pos->AttrPtr(3, gl::DataType::Float, GL_FALSE, sizeof(pointf3)));
    format.attrptrs.push_back(
        uv->AttrPtr(2, gl::DataType::Float, GL_FALSE, sizeof(pointf2)));
    format.attrptrs.push_back(
        normal->AttrPtr(3, gl::DataType::Float, GL_FALSE, sizeof(normalf)));
    format.attrptrs.push_back(
        tangent->AttrPtr(3, gl::DataType::Float, GL_FALSE, sizeof(vecf3)));
    format.eb = indices;
    auto rec = new PrimitiveResource;
    rec->va = new gl::VertexArray{{0, 1, 2, 3}, format};
    rec->eb = indices;
    rec->n2vb["pos"] = pos;
    rec->n2vb["uv"] = uv;
    rec->n2vb["normal"] = normal;
    rec->n2vb["tangent"] = tangent;
    p2r[primitive] = rec;
    return rec->va;
  } else
    return nullptr;
}

void DeferredRenderer::Impl::ResizeBuffer(size_t width, size_t height) {
  if (this->width == width && this->height == height)
    return;

  tex0->SetImage(0, gl::PixelDataInternalFormat::Rgba32F, width, height,
                 gl::PixelDataFormat::Rgba, gl::PixelDataType::Float, nullptr);
  depth->SetStorage(gl::FramebufferInternalFormat::DepthComponent, width,
                    height);

  gb.Attach(gl::FramebufferAttachment::ColorAttachment0, tex0);
  gb.DrawBuffers({gl::ColorBuffer::ColorAttachment0});
  gb.Attach(gl::FramebufferAttachment::DepthAttachment, depth);

  this->width = width;
  this->height = height;
}

void DeferredRenderer::Impl::RenderImpl(Scene* scene, SObj* camObj,
                                        size_t width, size_t height) {
  ResizeBuffer(width, height);

  gb.Bind();
  gl::Enable(gl::Capability::DepthTest);
  gl::ClearColor({0.f, 0.f, 0.f, 0.f});
  gl::Clear(gl::BufferSelectBit::ColorBufferBit |
            gl::BufferSelectBit::DepthBufferBit |
            gl::BufferSelectBit::StencilBufferBit);
  auto camera = camObj->Get<Cmpt::Camera>();
  assert(camera != nullptr);
  auto cam_l2w = camObj->Get<Cmpt::Transform>()->GetLocalToWorldMatrix();
  auto cam_pos = cam_l2w * pointf3{0.f};
  auto cam_front = cam_l2w * vecf3{0, 0, -1};
  gProgram->SetMatf4("view", transformf::look_at(cam_pos, cam_pos + cam_front));
  gProgram->SetMatf4("projection", transformf::perspective(
                                       camera->fov, camera->ar, 0.1f, 100.f));

  scene->Each([this](Cmpt::Geometry* geo) {
    Primitive* primitive = geo->primitive;
    auto va = GetPrimitiveVAO(primitive);
    if (!va)
      return;
    gProgram->SetMatf4(
        "model", geo->sobj->Get<Cmpt::Transform>()->GetLocalToWorldMatrix());
    va->Draw(gProgram);
  });

  gl::FrameBuffer::BindReset();
  gl::Disable(gl::Capability::DepthTest);
  screenProgram->Active(
      0, gb.GetTex2D(gl::FramebufferAttachment::ColorAttachment0));
  screen->va->Draw(screenProgram);
}
