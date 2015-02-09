#ifndef _ENTITY_H_
#define _ENTITY_H_
#include <list>
#include <stack>
#include <string>
#include "basic.h"
#include "solidbody.h"
#include "modelinstance.h"
#include "entityevent.h"
#include "world_graphics.h"
#include "world_physics.h"
#include "world.h"

class Entity;
class World;

typedef void (*stateCallback)(EntityEvent*,Entity*);
typedef void STATE;

//TODO: add ability to copy both position and orientation from BODY to MODEL

class EntityState
{
public:
	EntityState(stateCallback eventCallback, Entity* _caller, float wait=0.0f, int returnIndex = 0)
	{
		assert(eventCallback != NULL && _caller != NULL && wait >= 0.0f);
		callback = eventCallback;
		caller = _caller;
		waitTime = wait; //if wait == 0.0, EventTimer is disabled
		retIndBackup = returnIndex; //retInd should acquire it's true value only after EventTimer!
		retInd = 0;
		obsolete = false;
		holdEx = false;
	}
	~EntityState()
	{
	}
	void	handleEvents(std::list<EntityEvent*> &events)
	{
		if(waitTime > 0.0f) waitTime -= g_Delta;
		if(waitTime < 0.0f){ retInd = retIndBackup; events.push_back(new EventTimer()); }
		while(!events.empty())
		{
			callback(events.front(), caller);
			if(obsolete)
			{ //happens when current state is popped
				if(events.front()->eventCode == EventCode_Timer)
				{
					delete events.front();
					events.pop_front();
				}
				return;
			}
			delete events.front();
			events.pop_front();
			if(holdEx) { holdEx = false; return; }
		}
	}

	//when event pushes another state, the previous one should
	//stop proccessing events and leave them for the new state
	void	holdExecution() { holdEx = true; }
	void	setObsolete() { obsolete = true; }

	int		getReturnIndex() const { return retInd; }
private:
	Entity*						caller;
	stateCallback				callback;
	float						waitTime;
	int							retInd;
	int							retIndBackup;
	bool						obsolete;
	bool						holdEx;
};

class Entity : public Unique, public Touchable, public FamilyTree
{
public:
			 Entity();
	virtual ~Entity();
	virtual void			initialize();

	void					destroy();

	void					sendEvent(EntityEvent* ee);

	void					switchToEditorModel();
	void					switchToModel();

	std::string				getName() const { return name; }
	const SolidBody*		getBody() const { return body; }//for moving purposes
	const ModelInstance*	getModelInstance() const { return model; }//for anim playing
	glm::vec3				getDesiredLDir() const { return desiredLinearDirection; }
	glm::vec3				getDesiredADir() const { return desiredAngularDirection; }
	glm::vec3				getDesiredRotation() const { return desiredRotation; }
	float					getRotationSpeed() const { return rotationSpeed; }

	void					setName(std::string newName) { name = newName; }
	void					setDesiredLDir(glm::vec3 dir) { desiredLinearDirection = dir; }
	void					setDesiredADir(glm::vec3 dir) { desiredAngularDirection = dir; }
	void					setDesiredRotation(glm::vec3 rot) { desiredRotation = rot; }
	void					setRotationSpeed(float speed) { if(speed >=0.0f) rotationSpeed = speed; }

protected:
	//states:
	static STATE dummy(EntityEvent* ee, Entity* caller);
	static STATE autowaitState(EntityEvent* ee, Entity* caller);


	void					setupModel(std::string vertShader, std::string fragShader,
									   std::string modelPath, std::string diffTexture,
									   std::string normTexture);
	void					setupCollision(float mass); //from model
	void					setupCollision(float mass, float radius); //sphere
	void					setupCollision(float mass, glm::vec3 halfExtents); //box
	void					autowait(float time, int returnIndex);
	void					pushState(stateCallback callback, float waitTime = 0.0f);
	void					replaceState(stateCallback callback, float waitTime = 0.0f);
	void					popState(EntityEvent* ee = NULL);
	void					syncEntitySpeed();
	void					update();

	std::stack<EntityState*>	statesObsolete;
	std::stack<EntityState*>	states;
	std::list<EntityEvent*>		events;

	glm::vec3					desiredLinearDirection;
	glm::vec3					desiredAngularDirection;
	glm::vec3					desiredRotation; //doesn't affect physics, just the visible part
	float						rotationSpeed;
	bool						editor;
	SolidBody*					body;
	ModelInstance*				model;
	std::string					name;

	WorldGraphics*				wldGFX; //fill theese
	WorldPhysics*				wldPHY; //from class World
	World*						wld;

	friend class World;
};

class LiveEntity : public Entity
{
public:
			 LiveEntity();
	virtual ~LiveEntity();

	virtual void initialize();

	virtual void receiveDamage(Entity* inflictor, float damage, glm::vec3 direction);
	float		 getHealth() const { return health; }

protected:
	float	health;
};

#endif