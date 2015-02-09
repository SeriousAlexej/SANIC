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

	void	registerKeyPress(int keyCode);
	void	registerKeyRelease(int keyCode);
	void	registerMousePress(int mouseButton);
	void	registerMouseRelease(int mouseButton);

	void	updateMouseDelta();

	bool	keyPressed(int keyCode) const;
	bool	mouseButtonPressed(int mouseButton) const;

private:
	sf::Window*		mainWindow;
	sf::Vector2u	windowSize;
	glm::vec2		mouseDelta;

	bool	keyStatesNew[101];
	bool	keyStatesOld[101];

	bool	mouseKeysNew[5];
	bool	mouseKeysOld[5];
};

#endif