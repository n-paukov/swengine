#pragma once

#include <functional>

#include <Engine/Components/GUI/GUIWidget.h>
#include <Engine/Components/GUI/Widgets/GUIText.h>
#include <Engine/Components/Graphics/RenderSystem/GraphicsContext.h>

class GUITextBox : public GUIWidget {
public:
	using KeyPressCallback = std::function<void(KeyboardKey)>;

public:
	GUITextBox(GraphicsContext* graphicsContext, Font* font);
	~GUITextBox();

	void setText(const std::string& text);
	std::string getText() const;

	void clear();

	void setColor(const vector3& color);
	void setColor(float r, float g, float b);

	void setColor(const vector4& color);
	void setColor(float r, float g, float b, float a);

	vector4 getColor() const;

	void setFont(Font* font);
	Font* getFont() const;

	void setFontSize(unsigned int size);
	unsigned int getFontSize() const;

	void setPaddingTop(unsigned int paddingTop);
	unsigned int getPaddingTop() const;

	void setPaddingLeft(unsigned int paddingLeft);
	unsigned int getPaddingLeft() const;

	virtual void setPosition(const uivector2& position);
	virtual void setPosition(uint32 x, uint32 y);

	virtual void render(GeometryInstance* quad, GpuProgram* program) override;
	virtual void update(const CursorPosition& mousePosition) override;

	void setBackgroundColor(const vector4& color);
	void setBackgroundColor(float r, float g, float b, float a);

	vector4 getBackgroundColor() const;

	virtual void onKeyPress(KeyboardKey key) override;
	virtual void onKeyRepeat(KeyboardKey key) override;
	virtual void onCharacterEntered(unsigned char character) override;

	virtual void onKeyPress(const KeyPressCallback& callback);
protected:
	GUIText * m_text;

	unsigned int m_paddingTop;
	unsigned int m_paddingLeft;

	vector4 m_backgroundColor;

protected:
	KeyPressCallback m_keyPressCallback;

protected:
	GraphicsContext* m_graphicsContext;
};