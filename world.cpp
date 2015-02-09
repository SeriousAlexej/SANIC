#include "world.h"

World::World()
{
}

World::~World()
{
	for(int i=entities.size()-1; i>=0; i--)
	{
		delete entities[i];
	}
	entities.clear();
}

void World::update()
{
	for(int i=entities.size()-1; i>=0; i--)
	{
		entities[i]->update();
	}
	physics.update();
	graphics.render();
	//debugging
	physics.render(graphics.getCamera());
}

Entity* World::createEntity(Entity* e)
{
	//can't add already added or null entity
	assert(e != NULL && e->wldGFX == NULL && e->wldPHY == NULL && e->wld == NULL);
	e->wldGFX = &graphics;
	e->wldPHY = &physics;
	e->wld = this;
	entities.push_back(e);
	return e;
}

void World::removeEntity(Entity *e)
{
	if(e)
	{
		for(int i=entities.size()-1; i>=0; i--)
		{
			if(entities[i]==e)
			{
				delete e;
				entities.erase(entities.begin() + i);
				return;
			}
		}
	}
}
