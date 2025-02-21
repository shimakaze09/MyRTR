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
class Image;

class DeferredRenderer::Impl {
 public:
  Impl();
  ~Impl();

  void RenderImpl(Scene* scene, SObj* camObj, size_t width, size_t height);

 private:
  void ResizeBuffer(size_t width, size_t height);
  gl::VertexArray* GetPrimitiveVAO(const Primitive* primitive);
  enum class DefaultTex {
    White,
    Normal,
  };
  gl::Texture2D* GetTex2D(const Image* img,
                          DefaultTex default_tex = DefaultTex::White);

 private:
  struct PrimitiveResource;
  std::map<const Primitive*, PrimitiveResource*> p2r;
  size_t width{0};
  size_t height{0};
  std::array<gl::Texture2D*, 4> gtexs{nullptr};
  gl::RenderBuffer* depth{nullptr};
  gl::Program* gProgram{nullptr};
  gl::Program* deferredlightProgram{nullptr};
  gl::Program* screenProgram{nullptr};
  gl::FrameBuffer gb;
  std::map<std::string, gl::Shader*> path2shader;
  PrimitiveResource* screen{nullptr};
  PrimitiveResource* sphere{nullptr};
  gl::Texture2D default_white;
  gl::Texture2D default_normal;
};
}  // namespace My
