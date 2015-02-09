#ifndef _WORLD_H_
#define _WORLD_H_
#include "world_graphics.h"
#include "world_physics.h"
#include "entity.h"

class Entity;

class World
{
public:
	World();
	~World();

	void	update();
	Entity*	createEntity(Entity* e);
	void	removeEntity(Entity* e);

private:
	std::vector<Entity*>	entities;
	WorldGraphics			graphics;
	WorldPhysics			physics;
};

#endif