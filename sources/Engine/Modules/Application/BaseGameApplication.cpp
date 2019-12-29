#include "BaseGameApplication.h"

#include <Exceptions/EngineRuntimeException.h>
#include <spdlog/spdlog.h>

#include "Modules/Graphics/GUI/GUIConsole.h"

BaseGameApplication::BaseGameApplication(int argc, char* argv[], const std::string& windowTitle, int windowWidth, int windowHeight)
    : m_mainWindow(nullptr)
{
    spdlog::info("Application start...");

    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    int initStatus = SDL_Init(SDL_INIT_EVERYTHING);

    if (initStatus != 0) {
        ENGINE_RUNTIME_ERROR(std::string(SDL_GetError()));
    }

    spdlog::info("SDL is initialized");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    spdlog::info("Create main window...");

    m_mainWindow = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight, SDL_WINDOW_OPENGL);

    if (m_mainWindow == nullptr) {
        ENGINE_RUNTIME_ERROR(std::string(SDL_GetError()));
    }

    spdlog::info("Window is created");

    spdlog::info("Initialize engine modules...");

    m_graphicsModule = std::make_shared<GraphicsModule>(m_mainWindow);

    m_resourceManagementModule = std::make_shared<ResourceManagementModule>();

    std::shared_ptr<ResourceManager> resourceManager = m_resourceManagementModule->getResourceManager();
    resourceManager->declareResourceType<ShaderResource>("shader");
    resourceManager->declareResourceType<MeshResource>("mesh");
    resourceManager->declareResourceType<TextureResource>("texture");
    resourceManager->declareResourceType<BitmapFontResource>("bitmap_font");

    resourceManager->addResourcesMap("../resources/engine_resources.xml");

    std::shared_ptr<GLShader> guiVertexShader = resourceManager->
            getResourceFromInstance<ShaderResource>("gui_vertex_shader")->getShader();

    std::shared_ptr<GLShader> guiFragmentShader = resourceManager->
            getResourceFromInstance<ShaderResource>("gui_fragment_shader")->getShader();

    std::shared_ptr<GLShadersPipeline> guiShadersPipeline = std::make_shared<GLShadersPipeline>(
                guiVertexShader, guiFragmentShader, nullptr);

    m_inputModule = std::make_shared<InputModule>(m_mainWindow);

    m_gameWorld = std::make_shared<GameWorld>();

    m_inputSystem = std::make_shared<InputSystem>(m_gameWorld, m_inputModule);
    m_gameWorld->addGameSystem(m_inputSystem);

    m_sharedGraphicsState = std::make_shared<SharedGraphicsState>();
    m_meshRenderingSystem = std::make_shared<MeshRenderingSystem>(m_graphicsModule->getGraphicsContext(),
                                                                  m_sharedGraphicsState);
    m_gameWorld->addGameSystem(m_meshRenderingSystem);

    m_guiSystem = std::make_shared<GUISystem>(m_gameWorld, m_inputModule,
        m_graphicsModule->getGraphicsContext(), guiShadersPipeline);

    std::shared_ptr<BitmapFont> guiDefaultFont = resourceManager->
            getResourceFromInstance<BitmapFontResource>("gui_default_font")->getFont();
    m_guiSystem->setDefaultFont(guiDefaultFont);

    m_gameWorld->addGameSystem(m_guiSystem);

    m_screenManager = std::make_shared<ScreenManager>(m_gameWorld, m_graphicsModule,
                                                      m_sharedGraphicsState, resourceManager);

    m_guiSystem->setActiveLayout(m_screenManager->getCommonGUILayout());

    m_gameConsole = std::make_shared<GameConsole>(m_gameWorld);

    std::shared_ptr<GUIConsole> guiConsole = std::make_shared<GUIConsole>(m_gameConsole, 20, m_guiSystem->getDefaultFont());
    m_gameConsole->setGUIConsole(guiConsole);

    glm::vec4 guiConsoleBackgroundColor = { 0.168f, 0.172f, 0.25f, 0.8f };

    guiConsole->setBackgroundColor(guiConsoleBackgroundColor);
    guiConsole->setHoverBackgroundColor(guiConsoleBackgroundColor);
    guiConsole->setWidth(m_guiSystem->getScreenWidth());

    glm::vec4 guiConsoleTextBoxBackgroundColor = { 0.118f, 0.112f, 0.15f, 1.0f };

    guiConsole->getTextBox()->setBackgroundColor(guiConsoleTextBoxBackgroundColor);
    guiConsole->getTextBox()->setHoverBackgroundColor(guiConsoleTextBoxBackgroundColor);
    guiConsole->getTextBox()->setFocusBackgroundColor(guiConsoleTextBoxBackgroundColor);
    guiConsole->getTextBox()->setTextColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    guiConsole->getTextBox()->setTextHoverColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    guiConsole->getTextBox()->setTextFontSize(9);

    guiConsole->setTextFontSize(9);
    guiConsole->setTextColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    guiConsole->setTextHoverColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    guiConsole->setZIndex(10);
    guiConsole->hide();

    m_screenManager->getCommonGUILayout()->addChildWidget(guiConsole);

    m_gameWorld->subscribeEventsListener<GameConsoleCommandEvent>(this);
    m_gameWorld->subscribeEventsListener<InputActionToggleEvent>(this);

    m_inputModule->registerAction("console", KeyboardInputAction(SDLK_BACKQUOTE));

    DebugPainter::initialize(m_resourceManagementModule->getResourceManager(), m_sharedGraphicsState);

    m_gameConsole->print("Engine is initialized...");

    spdlog::info("Engine modules are initialized");
}

BaseGameApplication::~BaseGameApplication()
{
    SDL_DestroyWindow(m_mainWindow);
}

void BaseGameApplication::load()
{

}

void BaseGameApplication::unload()
{

}

void BaseGameApplication::update(float delta)
{
    ARG_UNUSED(delta);
}

void BaseGameApplication::render()
{

}

int BaseGameApplication::execute()
{
    spdlog::info("Perform game application loading...");
    performLoad();
    spdlog::info("Game application is loaded and ready...");

    SDL_ShowWindow(m_mainWindow);

    const int FRAMES_PER_SECOND = 30;
    const int SKIP_TICKS = 1000 / FRAMES_PER_SECOND;

    unsigned long nextTick = GetTickCount();

    long sleepTime = 0;

    SDL_Event event;

    spdlog::info("Starting main loop...");

    m_isMainLoopActive = true;

    while (m_isMainLoopActive) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE))
            {
                m_isMainLoopActive = false;
                break;
            }

            m_inputModule->processRawSDLEvent(event);
        }

        if (!m_isMainLoopActive)
            break;

        performUpdate(1.0f / FRAMES_PER_SECOND);

        if (!m_isMainLoopActive)
            break;

        performRender();

        nextTick += SKIP_TICKS;
        sleepTime = static_cast<long>(nextTick) - static_cast<long>(GetTickCount());

        if (sleepTime >= 0) {
            SDL_Delay(static_cast<unsigned long>(sleepTime));
        }
    }

    spdlog::info("Perform game application unloading...");
    performUnload();
    spdlog::info("Game application is unloaded...");

    return 0;
}

EventProcessStatus BaseGameApplication::receiveEvent(GameWorld* gameWorld, const GameConsoleCommandEvent& event)
{
    ARG_UNUSED(gameWorld);

    if (event.command == "exit") {
        shutdown();

        return EventProcessStatus::Prevented;
    }

    return EventProcessStatus::Processed;
}

EventProcessStatus BaseGameApplication::receiveEvent(GameWorld* gameWorld, const InputActionToggleEvent& event)
{
    ARG_UNUSED(gameWorld);

    if (event.actionName == "console" && event.newState == InputActionState::Active) {
        if (m_gameConsole->getGUIConsole()->isShown()) {
            m_gameConsole->getGUIConsole()->hide();
        }
        else {
            m_gameConsole->getGUIConsole()->show();
        }
    }

    return EventProcessStatus::Processed;
}

void BaseGameApplication::shutdown()
{
    m_isMainLoopActive = false;
}

void BaseGameApplication::performLoad()
{
    load();
}

void BaseGameApplication::performUnload()
{
    unload();

    m_screenManager.reset();

    SDL_Quit();
}

void BaseGameApplication::performUpdate(float delta)
{
    m_gameWorld->update(delta);
    m_screenManager->update(delta);

    update(delta);
}

void BaseGameApplication::performRender()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_gameWorld->render();
    m_screenManager->render();

    render();

    DebugPainter::flushRenderQueue(m_graphicsModule->getGraphicsContext().get());
    m_graphicsModule->getGraphicsContext()->swapBuffers();
}
