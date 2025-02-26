//
// Created by Admin on 21/02/2025.
//

#pragma once

#include <MyRTR/DeferredRenderer.h>

#include <MyGL/MyGL.h>

#include <map>
#include <string>

namespace My {
class Primitive;
class Texture2D;

class DeferredRenderer::Impl {
 public:
  Impl();
  ~Impl();

  void RenderImpl(Scene* scene, SObj* camObj, size_t width, size_t height);

 private:
  void ResizeBuffer(size_t width, size_t height);
  gl::Mesh* GetPrimitiveMesh(const Primitive* primitive);
  enum class DefaultTex {
    White,
    Normal,
  };
  gl::Texture2D* GetGLTex2D(const Texture2D* img,
                            DefaultTex default_tex = DefaultTex::White);

 private:
  std::map<const Primitive*, gl::Mesh*> primitive2mesh;
  size_t width{0};
  size_t height{0};
  std::array<gl::Texture2D*, 4> gtexs{nullptr};
  gl::RenderBuffer* depth{nullptr};
  gl::Program* gProgram{nullptr};
  gl::Program* deferredlightProgram{nullptr};
  gl::Program* screenProgram{nullptr};
  gl::FrameBuffer gb;
  gl::Texture2D* lightingBuffer_tex;
  gl::FrameBuffer lightingBuffer;
  gl::Mesh* screen{nullptr};
  gl::Mesh* sphere{nullptr};
  gl::Texture2D default_white;
  gl::Texture2D default_normal;
};
}  // namespace My
