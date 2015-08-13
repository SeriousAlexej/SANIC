#ifndef EDITOR_H
#define EDITOR_H
#include "world.h"
#include "input_handler.h"

class Editor
{

private:
    World* p_world;
	InputHandler* p_input;
    RayCastInfo castRayScreen(bool fromCenter = false);
public:
    Editor(sf::Window* w, World* wld);
    ~Editor();
	void update();
	InputHandler *getInputHandler() const { return p_input; }
	
	//editor stuff
	enum EditorMode { Fly, Moving, Pulling, Idle };

	Entity*					selectedEntity;
	float					editorFlySpeed;
	glm::vec2               mposOffsetMoving;

	TwBar*					entitiesList;
	EditorMode 				edMode;
};

#endif // EDITOR_H
