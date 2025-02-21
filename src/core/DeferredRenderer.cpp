//
// Created by Admin on 21/02/2025.
//

#include "Impl/DeferredRendererImpl.h"

using namespace My;
using namespace std;

DeferredRenderer::DeferredRenderer()
    : pImpl{make_unique<DeferredRenderer::Impl>()} {}

DeferredRenderer::~DeferredRenderer() = default;
DeferredRenderer::DeferredRenderer(DeferredRenderer&& dr) noexcept = default;
DeferredRenderer& DeferredRenderer::operator=(DeferredRenderer&& dr) noexcept =
    default;

void DeferredRenderer::Render(Scene* scene, SObj* camObj, size_t width,
                              size_t height) {
  Pimpl()->RenderImpl(scene, camObj, width, height);
}
