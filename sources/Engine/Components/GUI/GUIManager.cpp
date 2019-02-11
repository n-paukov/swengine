#include "GUIManager.h"

#include <algorithm>
#include <iostream>

GUIManager::GUIManager(Window * window, 
	InputManager* inputManager, 
	GraphicsContext* graphicsContext, 
	GpuProgram* guiGpuProgram)
	: m_window(window), 
	m_inputManager(inputManager), 
	m_graphicsContext(graphicsContext),
	m_gpuProgram(guiGpuProgram),
	m_quad(nullptr),
	m_mainLayout(new GUILayout()),
	m_focusedWidget(nullptr)
{
	m_onFocusCallback = std::bind(&GUIManager::onFocus, this, std::placeholders::_1);

	m_inputManager->registerEventListener(this);

	// position (x,y), texture coordinates
	float quadVertices[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	m_quad = m_graphicsContext->createGeometryInstance();
	m_quad->setVerticesData(6, sizeof(quadVertices), (const std::byte*)quadVertices, GeometryInstance::DataUsage::StaticDraw);

	// position and texture coordinates attribute
	m_quad->setAttributeDesc(0, GeometryAttributeDesc(GeometryAttributeType::Float, 0, 4, 4 * sizeof(float)));

	m_quad->create();

	m_mainLayout->setPosition(0, 0);
	m_mainLayout->setSize(m_window->getWidth(), m_window->getHeight());
	m_mainLayout->setPosition(0, 0);

	m_mainLayout->setOnFocusCallback(m_onFocusCallback);

	m_orthographicProjection = glm::ortho(0.0f, (float)m_window->getWidth(), (float)m_window->getHeight(), 0.0f, -1.0f, 1.0f);
}

GUIManager::~GUIManager()
{
	delete m_mainLayout;
	delete m_quad;
}

void GUIManager::render()
{
	m_graphicsContext->getWindowRenderTarget()->bind();

	m_graphicsContext->disableDepthTest();
	m_graphicsContext->enableBlending();
	m_graphicsContext->setBlendingMode(GraphicsContext::BlendingMode::SrcAlpha, 
		GraphicsContext::BlendingMode::OneMinusSrcAlpha);

	m_quad->bind();
	m_gpuProgram->bind();
	m_gpuProgram->setParameter("transform.projection", m_orthographicProjection);

	if (m_mainLayout->isVisible())
		m_mainLayout->render(m_quad, m_gpuProgram);
}

void GUIManager::update()
{
	bool needTriggerMouseEvents = getCursorMode() == CursorMode::Default;

	MousePosition mousePosition = m_inputManager->getMousePosition();

	if (needTriggerMouseEvents) {
		m_mainLayout->onHover(mousePosition);
	}

	m_mainLayout->update(mousePosition);
}

GUILayout * GUIManager::getMainLayout() const
{
	return m_mainLayout;
}

void GUIManager::setCursorMode(CursorMode mode)
{
	m_window->setCursorMode(mode);
}

CursorMode GUIManager::getCursorMode() const
{
	return m_window->getCursorMode();
}

void GUIManager::resetCurrentCursor() {

}

void GUIManager::setCurrentCursor(Cursor * cursor)
{
	m_window->setCurrentCursor(cursor);
}

Cursor * GUIManager::getCurrentCursor() const
{
	return m_window->getCurrentCursor();
}

void GUIManager::onMouseButtonPress(MouseButton button, const MouseState &)
{
	bool needTriggerMouseEvents = getCursorMode() == CursorMode::Default;

	if (!needTriggerMouseEvents)
		return;

	MousePosition mousePosition = m_inputManager->getMousePosition();

	m_mainLayout->onClick(mousePosition, button);
}

void GUIManager::onKeyPress(Key key, KeyEvent::Modifier mod)
{
	if (m_focusedWidget != nullptr)
		m_focusedWidget->onKeyPress(key, mod);
}

void GUIManager::onCharacterEntered(unsigned char character)
{
	if (m_focusedWidget != nullptr)
		m_focusedWidget->onCharacterEntered(character);
}

void GUIManager::onKeyRepeat(Key key, KeyEvent::Modifier mod)
{
	if (m_focusedWidget != nullptr)
		m_focusedWidget->onKeyRepeat(key, mod);
}

void GUIManager::onFocus(GUIWidget * widget)
{
	m_focusedWidget = widget;
}
