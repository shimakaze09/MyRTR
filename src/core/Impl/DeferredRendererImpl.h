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
  enum class TexPrecision { Byte8, F16, F32 };
  gl::Texture2D* GetGLTex2D(const Texture2D* img,
                            DefaultTex default_tex = DefaultTex::White,
                            TexPrecision precision = TexPrecision::Byte8);

 private:
  size_t width{0};
  size_t height{0};

  gl::Texture2D default_white;
  gl::Texture2D default_normal;

  gl::Texture2D LTC_tsfm;
  gl::Texture2D LTC_nf0s;

  std::map<const Primitive*, gl::Mesh*> primitive2mesh;
  gl::Mesh* screen{nullptr};
  gl::Mesh* sphere{nullptr};
  gl::Mesh* cube{nullptr};
  gl::Mesh* square{nullptr};

  gl::Program* stdBRDFProgram{nullptr};
  gl::Program* lightProgram{nullptr};
  gl::Program* deferred_lightingProgram{nullptr};
  gl::Program* envProgram{nullptr};
  gl::Program* postprocessProgram{nullptr};
  gl::Program* screenProgram{nullptr};

  std::array<gl::Texture2D*, 4> gtexs{nullptr};
  gl::RenderBuffer* depth{nullptr};
  gl::FrameBuffer gb;

  gl::Texture2D* lightingBuffer_tex;
  gl::FrameBuffer lightingBuffer;
};
}  // namespace My
