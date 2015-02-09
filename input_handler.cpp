#include <cstring> //memset
#include "input_handler.h"

InputHandler::InputHandler(sf::Window* w)
{
	assert(w != NULL);
	mainWindow = w;
	memset(keyStatesNew, 0, sizeof(bool)*101);
	memset(keyStatesOld, 0, sizeof(bool)*101);
	memset(mouseKeysNew, 0, sizeof(bool)*5);
	memset(mouseKeysOld, 0, sizeof(bool)*5);
	mouseDelta.x = 0.0f;
	mouseDelta.y = 0.0f;
	windowSize = mainWindow->getSize();
}

InputHandler::~InputHandler()
{
}

void InputHandler::registerKeyPress(int keyCode)
{
	if(keyCode >=0 && keyCode <= 100)
	{
		bool oldValue = keyStatesNew[keyCode];
		keyStatesOld[keyCode] = oldValue;
		keyStatesNew[keyCode] = true;
	}
}

void InputHandler::registerMousePress(int mouseButton)
{
	if(mouseButton >=0 && mouseButton <= 4)
	{
		bool oldValue = mouseKeysNew[mouseButton];
		mouseKeysOld[mouseButton] = oldValue;
		mouseKeysNew[mouseButton] = true;
	}
}

void InputHandler::registerMouseRelease(int mouseButton)
{
	if(mouseButton >=0 && mouseButton <= 4)
	{
		bool oldValue = mouseKeysNew[mouseButton];
		mouseKeysOld[mouseButton] = oldValue;
		mouseKeysNew[mouseButton] = false;
	}
}

void InputHandler::registerKeyRelease(int keyCode)
{
	if(keyCode >=0 && keyCode <= 100)
	{
		bool oldValue = keyStatesNew[keyCode];
		keyStatesOld[keyCode] = oldValue;
		keyStatesNew[keyCode] = false;
	}
}

bool InputHandler::keyPressed(int keyCode) const
{
	assert(keyCode >=0 && keyCode <= 100);
	return keyStatesNew[keyCode];
}

bool InputHandler::mouseButtonPressed(int mouseButton) const
{
	assert(mouseButton >=0 && mouseButton <= 4);
	return mouseKeysNew[mouseButton];
}

void InputHandler::updateMouseDelta()
{
	sf::Vector2i mpos = sf::Mouse::getPosition(*mainWindow);
	mouseDelta.x = float(windowSize.x - mpos.x);
	mouseDelta.y = float(windowSize.y - mpos.y);
}
