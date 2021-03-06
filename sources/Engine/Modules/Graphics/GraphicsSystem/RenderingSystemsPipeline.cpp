#include "precompiled.h"

#pragma hdrstop

#include "RenderingSystemsPipeline.h"

#include <utility>
#include "FrameStats.h"
#include "DebugPainter.h"

RenderingSystemsPipeline::RenderingSystemsPipeline(
  std::shared_ptr<GLGraphicsContext> graphicsContext)
  : GameSystemsGroup(),
    m_graphicsContext(std::move(graphicsContext))
    {
}

void RenderingSystemsPipeline::addGameSystem(std::shared_ptr<GameSystem> system)
{
  SW_ASSERT(dynamic_cast<RenderingSystem*>(system.get()) != nullptr);

  GameSystemsGroup::addGameSystem(system);
}

void RenderingSystemsPipeline::render()
{
  for (auto& system : getGameSystems()) {
    auto* renderingSystem = dynamic_cast<RenderingSystem*>(system.get());
    renderingSystem->render();
  }
}

void RenderingSystemsPipeline::afterRender()
{
}
