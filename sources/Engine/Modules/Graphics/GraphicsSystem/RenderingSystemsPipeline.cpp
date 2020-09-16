#include "precompiled.h"

#pragma hdrstop

#include "RenderingSystemsPipeline.h"

#include <utility>
#include "SharedGraphicsState.h"
#include "DebugPainter.h"

RenderingSystemsPipeline::RenderingSystemsPipeline(std::shared_ptr<GLGraphicsContext> graphicsContext,
  std::shared_ptr<SharedGraphicsState> sharedGraphicsState)
  : GameSystemsGroup(),
    m_graphicsContext(std::move(graphicsContext)),
    m_sharedGraphicsState(std::move(sharedGraphicsState)),
    m_deferredAccumulationMaterial(std::make_shared<Material>(std::make_unique<GLMaterial>()))
{
  GLMaterial& gpuMaterial = m_deferredAccumulationMaterial->getGpuMaterial();
  gpuMaterial.setBlendingMode(BlendingMode::Disabled);
  gpuMaterial.setDepthTestMode(DepthTestMode::NotEqual);
  gpuMaterial.setDepthWritingMode(DepthWritingMode::Disabled);
  gpuMaterial.setFaceCullingMode(FaceCullingMode::Disabled);
  gpuMaterial.setPolygonFillingMode(PolygonFillingMode::Fill);
}

void RenderingSystemsPipeline::addGameSystem(std::shared_ptr<GameSystem> system)
{
  SW_ASSERT(dynamic_cast<RenderingSystem*>(system.get()) != nullptr);

  GameSystemsGroup::addGameSystem(system);
}

void RenderingSystemsPipeline::render()
{
  // TODO: get rid of buffers clearing and copying as possible
  // Use depth swap trick to avoid depth buffer clearing
  m_sharedGraphicsState->getDeferredFramebuffer().clearColor({0.0f, 0.0f, 0.0f, 0.0f}, 0);
  m_sharedGraphicsState->getDeferredFramebuffer().clearColor({0.0f, 0.0f, 0.0f, 0.0f}, 1);
  m_sharedGraphicsState->getDeferredFramebuffer().clearColor({0.0f, 0.0f, 0.0f, 0.0f}, 2);

  m_sharedGraphicsState->getDeferredFramebuffer().clearDepthStencil(1.0f, 0);

  for (auto& system : getGameSystems()) {
    auto* renderingSystem = dynamic_cast<RenderingSystem*>(system.get());
    renderingSystem->renderDeferred();
  }

  GLShadersPipeline* accumulationPipeline = m_deferredAccumulationMaterial->getGpuMaterial().getShadersPipeline().get();
  GLShader* accumulationFragmentShader = accumulationPipeline->getShader(GL_FRAGMENT_SHADER);
  const GLFramebuffer& deferredFramebuffer = m_sharedGraphicsState->getDeferredFramebuffer();

  accumulationFragmentShader->setParameter("gBuffer.albedo",
    *deferredFramebuffer.getColorComponent(0), 0);

  accumulationFragmentShader->setParameter("gBuffer.normals",
    *deferredFramebuffer.getColorComponent(1), 1);

  accumulationFragmentShader->setParameter("gBuffer.positions",
    *deferredFramebuffer.getColorComponent(2), 2);

  m_graphicsContext->executeRenderTask(RenderTask{
    &m_deferredAccumulationMaterial->getGpuMaterial(),
    &m_graphicsContext->getNDCTexturedQuad(),
    0, 6,
    GL_TRIANGLES,
    &m_sharedGraphicsState->getForwardFramebuffer()
  });

  for (auto& system : getGameSystems()) {
    auto* renderingSystem = dynamic_cast<RenderingSystem*>(system.get());
    renderingSystem->renderForward();
  }

  for (auto& system : getGameSystems()) {
    auto* renderingSystem = dynamic_cast<RenderingSystem*>(system.get());
    renderingSystem->renderPostProcess();
  }

  m_graphicsContext->getDefaultFramebuffer().clearColor({0.0f, 0.0f, 0.0f, 1.0f});
  m_graphicsContext->getDefaultFramebuffer().clearDepthStencil(0.0f, 0);

  m_sharedGraphicsState->getForwardFramebuffer().copyColor(m_graphicsContext->getDefaultFramebuffer());
  m_sharedGraphicsState->getForwardFramebuffer().copyDepthStencil(m_graphicsContext->getDefaultFramebuffer());

  DebugPainter::flushRenderQueue(m_graphicsContext.get());
}

void RenderingSystemsPipeline::setDeferredAccumulationShadersPipeline(std::shared_ptr<GLShadersPipeline> pipeline)
{
  m_deferredAccumulationMaterial->getGpuMaterial().setShadersPipeline(std::move(pipeline));
}

std::shared_ptr<GLShadersPipeline> RenderingSystemsPipeline::getDeferredAccumulationShadersPipeline() const
{
  return m_deferredAccumulationMaterial->getGpuMaterial().getShadersPipeline();
}
