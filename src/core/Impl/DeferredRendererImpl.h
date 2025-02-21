//
// Created by Admin on 21/02/2025.
//

#pragma once

#include <MyRTR/DeferredRenderer.h>

#include <MyGL/MyGL>

#include <map>
#include <string>

namespace My {
class Primitive;

class DeferredRenderer::Impl {
 public:
  Impl();
  ~Impl();

  void RenderImpl(Scene* scene, SObj* camObj, size_t width, size_t height);

 private:
  void ResizeBuffer(size_t width, size_t height);
  gl::VertexArray* GetPrimitiveVAO(const Primitive* primitive);

 private:
  struct PrimitiveResource {
    gl::VertexArray* va{nullptr};
    std::map<std::string, gl::VertexBuffer*> n2vb;
    gl::ElementBuffer* eb{nullptr};

    ~PrimitiveResource() {
      delete va;
      for (const auto& [name, vb] : n2vb)
        delete vb;
      delete eb;
    }
  };

  std::map<const Primitive*, PrimitiveResource*> p2r;
  size_t width{0};
  size_t height{0};
  gl::Texture2D* tex0{nullptr};
  gl::Texture2D* tex1{nullptr};
  gl::Texture2D* tex2{nullptr};
  gl::Texture2D* tex3{nullptr};
  gl::RenderBuffer* depth{nullptr};
  gl::Program* gProgram{nullptr};
  gl::Program* screenProgram{nullptr};
  gl::FrameBuffer gb;
  std::map<std::string, gl::Shader*> path2shader;
  PrimitiveResource* screen{nullptr};
};
}  // namespace My
