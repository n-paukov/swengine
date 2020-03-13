#include "DemoApplication.h"

#include <spdlog/spdlog.h>
#include <Engine/Exceptions/EngineRuntimeException.h>
#include <Engine/Modules/Graphics/Resources/SkeletonResource.h>

#include "Game/Screens/GameScreen.h"
#include "Game/Screens/MainMenuScreen.h"

#include <glm/gtx/string_cast.hpp>

DemoApplication::DemoApplication(int argc, char* argv[])
    : BaseGameApplication(argc, argv, "Game", 1280, 720)
{

}

DemoApplication::~DemoApplication()
{

}

void DemoApplication::render()
{
}

void DemoApplication::load()
{
    auto resourceMgr = m_resourceManagementModule->getResourceManager();
    resourceMgr->addResourcesMap("../resources/resources.xml");

    m_screenManager->registerScreen(std::make_shared<GameScreen>(m_inputModule));
    m_screenManager->registerScreen(std::make_shared<MainMenuScreen>(m_inputModule, m_gameConsole));

    m_screenManager->changeScreen(BaseGameScreen::getScreenName(GameScreenType::MainMenu));

    std::shared_ptr deferredAccumulationPipeline = std::make_shared<GLShadersPipeline>(
        resourceMgr->getResourceFromInstance<ShaderResource>("deferred_accum_pass_vertex_shader")->getShader(),
        resourceMgr->getResourceFromInstance<ShaderResource>("deferred_accum_pass_fragment_shader")->getShader(),
        nullptr);

    m_renderingSystemsPipeline->setDeferredAccumulationShadersPipeline(deferredAccumulationPipeline);
}
