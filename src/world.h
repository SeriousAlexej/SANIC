#ifndef _WORLD_H_
#define _WORLD_H_
#include "world_physics.h"
#include "world_graphics.h"
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
	friend class Editor;
public:
	World();
	~World();

	void	update();
	void	updateEditor();
	Entity* createEntity(std::string entityName);
	void	removeEntity(Entity* e);

	RayCastInfo	castRay(glm::vec3 origin, glm::vec3 direction);

    void Save(const std::string& filename);
    void Love(const std::string& filename); // Lyubov porojdaet mir

private:
	std::vector<Entity*>	entities;
	WorldGraphics*			pGraphics;   // Sometimes we don't need graphics
	WorldPhysics			physics;

private:

	Entity*	createEntity(Entity* e);
};

#endif
