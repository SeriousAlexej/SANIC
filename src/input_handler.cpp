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
	wheelDelta = 0;
	lockMouse = false;
	windowSize = mainWindow->getSize();
	allowCheck = true;
	focus = true;
}

InputHandler::~InputHandler()
{
}

void InputHandler::registerKeyPress(int keyCode)
{
		keyStatesOld[keyCode] = false;
		keyStatesNew[keyCode] = true;
}

void InputHandler::registerMousePress(int mouseButton)
{
		mouseKeysOld[mouseButton] = false;
		mouseKeysNew[mouseButton] = true;
}

void InputHandler::registerMouseRelease(int mouseButton)
{
		mouseKeysOld[mouseButton] = true;
		mouseKeysNew[mouseButton] = false;
}

void InputHandler::registerKeyRelease(int keyCode)
{
		keyStatesOld[keyCode] = true;
		keyStatesNew[keyCode] = false;
		if(keyCode == sf::Keyboard::Escape)
		{
			lockMouse = !lockMouse;
			mainWindow->setMouseCursorVisible(!lockMouse);
			if(lockMouse)
			{
                mouseOldPosition = sf::Mouse::getPosition(*mainWindow);
				sf::Mouse::setPosition(sf::Vector2i(windowSize.x/2,windowSize.y/2), *mainWindow);
			} else {
				mouseDelta.x = 0.0f;
				mouseDelta.y = 0.0f;
				sf::Mouse::setPosition(mouseOldPosition, *mainWindow);
			}
		}
}

void InputHandler::registerWheelDelta(int delta)
{
	wheelDelta = delta;
}

bool InputHandler::keyJustPressed(int keyCode) const
{
	assert(keyCode >=0 && keyCode <= 100);
	return keyStatesNew[keyCode] && !keyStatesOld[keyCode];
}

bool InputHandler::keyJustReleased(int keyCode) const
{
	assert(keyCode >=0 && keyCode <= 100);
	return !keyStatesNew[keyCode] && keyStatesOld[keyCode];
}

bool InputHandler::keyPressed(int keyCode) const
{
	assert(keyCode >=0 && keyCode <= 100);
	return keyStatesNew[keyCode];
}

bool InputHandler::mouseButtonJustPressed(int mouseButton) const
{
	assert(mouseButton >=0 && mouseButton <= 4);
	return mouseKeysNew[mouseButton] && !mouseKeysOld[mouseButton];
}

bool InputHandler::mouseButtonJustReleased(int mouseButton) const
{
	assert(mouseButton >=0 && mouseButton <= 4);
	return !mouseKeysNew[mouseButton] && mouseKeysOld[mouseButton];
}

bool InputHandler::mouseButtonPressed(int mouseButton) const
{
	assert(mouseButton >=0 && mouseButton <= 4);
	return mouseKeysNew[mouseButton];
}

bool InputHandler::cursorIsInsideWindow() const
{
    sf::Vector2i mpos = sf::Mouse::getPosition(*mainWindow);
    return mpos.x > 0 && mpos.y > 0 && mpos.x < static_cast<int>(windowSize.x) && mpos.y < static_cast<int>(windowSize.y);
}

void InputHandler::setFocus(bool _f)
{
    focus = _f;
    lockMouse = false;
    mainWindow->setMouseCursorVisible(true);
	mouseDelta.x = 0.0f;
	mouseDelta.y = 0.0f;
}

bool InputHandler::isFocused() const
{
    return focus;
}

void InputHandler::update()
{
	if(allowCheck)
	{
        for(int i=0; i<101; i++)
        {
            bool pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key(i));
            if(!keyStatesNew[i] && pressed)
            {
                this->registerKeyPress(i);
            }
            else
            if(keyStatesNew[i] && !pressed)
            {
                this->registerKeyRelease(i);
            }
            else
            {
                keyStatesOld[i] = keyStatesNew[i];
            }
        }
        for(int i=0; i<5; i++)
        {
            bool pressed = sf::Mouse::isButtonPressed(sf::Mouse::Button(i));
            if(!mouseKeysNew[i] && pressed)
            {
                this->registerMousePress(i);
            }
            else
            if(mouseKeysNew[i] && !pressed)
            {
                this->registerMouseRelease(i);
            }
            else
            {
                mouseKeysOld[i] = mouseKeysNew[i];
            }
        }
	}
	if(lockMouse)
	{
		sf::Vector2i mpos = sf::Mouse::getPosition(*mainWindow);
		mouseDelta.x = float(windowSize.x/2 - float(mpos.x));
		mouseDelta.y = float(windowSize.y/2 - float(mpos.y));
		sf::Mouse::setPosition(sf::Vector2i(windowSize.x/2,windowSize.y/2), *mainWindow);
	}
}
