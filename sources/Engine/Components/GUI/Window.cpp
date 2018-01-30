#include "Window.h"
#include <iostream>
#include <Engine\Components\Debugging\Log.h>

Window::Window(const std::string& name, int width, int height) 
	: m_width(width),
	m_height(height),
	m_viewport(new Viewport(width, height))
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwInit();

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	this->m_windowPointer = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

	if (!this->m_windowPointer) {
		errlog() << "Failed to create GLFW window";
		glfwTerminate();
	}

	glfwMakeContextCurrent(this->m_windowPointer);

	glfwGetFramebufferSize(this->m_windowPointer, &width, &height);
	glViewport(0, 0, width, height);
}

Window::~Window() {

}

void Window::update() {
	if (m_cursorType == CursorType::Hidden) {
		resetCursorPosition();
	}
}

void Window::render() {
	
}

int16 Window::getWidth() const {
	return m_width;
}

int16 Window::getHeight() const {
	return m_height;
}

void Window::setCursorType(CursorType type) {
	m_cursorType = type;

	if (type == CursorType::Default) {
		glfwSetInputMode(m_windowPointer, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	else if (type == CursorType::Hidden) {
		glfwSetInputMode(m_windowPointer, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		resetCursorPosition();
	}
}

CursorType Window::getCursorType() const {
	return m_cursorType;
}

Viewport* Window::getViewport() const {
	return m_viewport;
}

void Window::setViewport(Viewport* viewport) {
	m_viewport = viewport;
}

void Window::setCursorPosition(real64 x, real64 y) {
	glfwSetCursorPos(m_windowPointer, x, y);
}

void Window::resetCursorPosition() {
	setCursorPosition(0, 0);
}

WindowCursorState Window::getCursorPosition() const {
	WindowCursorState state;

	glfwGetCursorPos(m_windowPointer, &state.x, &state.y);

	return state;
}

void Window::getCursorPosition(real64* x, real64* y) const {
	glfwGetCursorPos(m_windowPointer, x, y);
}

void Window::close() {
	glfwSetWindowShouldClose(this->m_windowPointer, GL_TRUE);
}

bool Window::shouldClose() const {
	return static_cast<bool>(glfwWindowShouldClose(this->m_windowPointer));
}

GLFWwindow* Window::getWindowPointer() const {
	return m_windowPointer;
}