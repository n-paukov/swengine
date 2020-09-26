#pragma once

#include <memory>

#include "Modules/ECS/GameSystemsGroup.h"
#include "RenderingSystem.h"
#include "Material.h"

// TODO: make rendering system more high-level and get rid of low-level primitives
//  usage here. High-level rendering system shouldn't organize forward, deferred, post-process
//  pipelines itself. Instead, it should create appropriate high-level render tasks,
//  probably, organize them in groups, so that low-level renderer can render each group without
//  changing of graphics context state.
//  High-level rendering system also should not manually manage graphics context state and
//  somehow switch its parameters. Instead, all such logic should be located in low-level renderer.
//  I.e. it should be possible to switch from deferred rendering to forward or back,
//  to add low-level optimization, to switch from OpenGL to DirectX, Vulkan and back without
//  modifying high-level systems.

class RenderingSystemsPipeline : public GameSystemsGroup {
 public:
  RenderingSystemsPipeline(
    std::shared_ptr<GLGraphicsContext> graphicsContext,
    std::shared_ptr<GraphicsScene> graphicsScene);

  void addGameSystem(std::shared_ptr<GameSystem> system) override;

  void render() override;

  void setDeferredAccumulationShadersPipeline(std::shared_ptr<GLShadersPipeline> pipeline);
  [[nodiscard]] std::shared_ptr<GLShadersPipeline> getDeferredAccumulationShadersPipeline() const;

 private:
  std::shared_ptr<GLGraphicsContext> m_graphicsContext;
  std::shared_ptr<SharedGraphicsState> m_sharedGraphicsState;

  std::shared_ptr<Material> m_deferredAccumulationMaterial;
};

