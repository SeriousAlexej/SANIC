#ifndef _WORLD_H_
#define _WORLD_H_
#include "input_handler.h"
#include "world_graphics.h"
#include "world_physics.h"
#include "entity.h"

class Entity;

struct RayCastInfo
{
public:
	Entity*		enHit;
	glm::vec3	posOrigin;
	glm::vec3   direction;
	glm::vec3	posHit;
	glm::vec3	normHit;
	float		rayLength;
};

class World
{
public:
	World(sf::Window* w);
	~World();

	void	update();
	void	updateEditor();
	Entity*	createEntity(Entity* e);
	void	removeEntity(Entity* e);

	RayCastInfo	castRay(glm::vec3 origin, glm::vec3 direction);
	RayCastInfo	castRayScreen(bool fromCenter = false);

	InputHandler *getInputHandler() const { return input; }

//private:
	InputHandler*			input;
	std::vector<Entity*>	entities;
	WorldGraphics			graphics;
	WorldPhysics			physics;

	//editor stuff
	enum EditorMode { Fly, Moving, Pulling, Idle };

	Entity*					selectedEntity;
	float					editorFlySpeed;
	glm::vec2               mposOffsetMoving;
	EditorMode              edMode;

	TwBar*					entitiesList;
};

#endif
