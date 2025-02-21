//
// Created by Admin on 21/02/2025.
//

#pragma once

#include <memory>

namespace My {
class Scene;
class SObj;

class DeferredRenderer {
 public:
  DeferredRenderer();
  ~DeferredRenderer();
  DeferredRenderer(DeferredRenderer&& dr) noexcept;
  DeferredRenderer& operator=(DeferredRenderer&& dr) noexcept;

  // [summary]
  // render scene to default framebuffer
  // [argument]
  // camObj must have Cmpt::Camera
  void Render(Scene* scene, SObj* camObj, size_t width, size_t height);

 private:
  class Impl;

  const Impl* Pimpl() const { return pImpl.get(); }

  Impl* Pimpl() { return pImpl.get(); }

  std::unique_ptr<Impl> pImpl;
};
}  // namespace My
