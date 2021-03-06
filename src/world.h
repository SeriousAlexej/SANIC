#ifndef _WORLD_H_
#define _WORLD_H_
#include "world_physics.h"
#include "world_graphics.h"

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
    friend class Game;
	friend class Editor;
public:
	World();
	~World();

	void	update();
	void	updateEditor();
    Entity* createEntity(const std::string &entityName);
    Entity* createEntity(std::shared_ptr<Entity> e);
	void	removeEntity(Entity* e);

	RayCastInfo	castRay(glm::vec3 origin, glm::vec3 direction);

    void Clear(); //complementation of entities
    void Save(const std::string& filename);
    void Love(const std::string& filename); // Lyubov porojdaet mir
    Entity* Paste(std::string& src);
    Entity* GetEntityWithID(int id);

    std::vector<std::shared_ptr<Entity>>& getEntities() { return entities; }

private:
    void deleteAllEntities(); //red button

	std::vector<std::shared_ptr<Entity>>	entities;
	std::vector<std::shared_ptr<Entity>>    obsoleteEntitties; // entities to be deleted upon next update, populated on Entity::destroy()
	WorldGraphics*			pGraphics;   // there are thigs possible to render with nothing but heart
	WorldPhysics			physics;
    void registerEntity(const std::string& name);
    void registerLua();
};

#endif
