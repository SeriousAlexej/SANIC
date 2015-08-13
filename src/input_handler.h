#ifndef _INPUT_HANDLER_H_
#define _INPUT_HANDLER_H_
#include <glm/glm.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window.hpp>

//TODO: add subscribers for receiving key events
class InputHandler
{
public:
	InputHandler(sf::Window* w);
	~InputHandler();

	bool	allowCheck;
	void	registerWheelDelta(int delta);

	void	update();

	bool	keyJustPressed(int keyCode) const;
	bool	keyJustReleased(int keyCode) const;
	bool	keyPressed(int keyCode) const;

	bool    cursorIsInsideWindow() const;

	bool	mouseButtonJustPressed(int mouseButton) const;
	bool	mouseButtonJustReleased(int mouseButton) const;
	bool	mouseButtonPressed(int mouseButton) const;

	void    setFocus(bool _f);
	bool    isFocused() const;

private:
	void	registerKeyPress(int keyCode);
	void	registerKeyRelease(int keyCode);
	void	registerMousePress(int mouseButton);
	void	registerMouseRelease(int mouseButton);

	sf::Window*		mainWindow;
	sf::Vector2u	windowSize;
	int				wheelDelta;
	glm::vec2		mouseDelta;
	sf::Vector2i    mouseOldPosition;//restore this position after exiting fly mode

	bool	lockMouse;

	bool	keyStatesNew[101];
	bool	keyStatesOld[101];

	bool	mouseKeysNew[5];
	bool	mouseKeysOld[5];

	bool    focus;

	//friend class World;
	friend class Editor;
};

#endif
