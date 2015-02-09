#include "entity.h"
#include "quaternion_utils.h"

STATE Entity::dummy(EntityEvent *ee, Entity* caller)
{
	//does nothing
	switch(ee->eventCode)
	{

	case EventCode_Begin:
		caller->switchToEditorModel();
		return; //this event was proccessed

	case EventCode_Dummy:
		printf("calling autowait\n");
		caller->autowait(3.0f, 1);
		return;

	case EventCode_AutowaitCallback:
		{
			EventAutowaitCallback* eac = (EventAutowaitCallback*)ee;
			printf("autowait returned %d!\n", eac->index);
			return;
		}

	default:
		return; //any event was 'proccessed'
	}

	//event was not proccessed
	caller->popState();
}

STATE Entity::autowaitState(EntityEvent *ee, Entity *caller)
{
	switch(ee->eventCode)
	{

	case EventCode_Timer:
		caller->popState(); //yeah, it's pointless, but you get the idea
		return; //this event was 'proccessed' x)

	}
	
	printf("autowait canceled due to unknown event with code %d\n", ee->eventCode);
	//event was not proccessed
	caller->popState();
}

Entity::Entity()
{
	wld = NULL;
	wldGFX = NULL;
	wldPHY = NULL;
	editor = false;
	body = NULL;
	model = NULL;
	pushState(dummy, 0);
	setClass("Entity");
	name = "";
	desiredLinearDirection = glm::vec3(0,0,0);
	desiredAngularDirection = glm::vec3(0,0,0);
	desiredRotation = glm::vec3(0,0,0);
	rotationSpeed = 1.0f;
}

Entity::~Entity()
{
	if(body)
	{
		assert(wldPHY);
		wldPHY->remBody(body);
	}
	if(model)
	{
		assert(wldGFX);
		wldGFX->deleteModel(model);
	}

	while(!statesObsolete.empty()) { delete statesObsolete.top(); statesObsolete.pop(); }
	while(!states.empty()) { delete states.top(); states.pop(); }
	while(!events.empty()) { delete events.front(); events.pop_front(); }
}

void Entity::sendEvent(EntityEvent *ee)
{
	if(ee)
	{
		//they say Bullet sends multiple CollisionCallbacks...
		//let's fix this by deleting duplicate touch events!
		if(ee->eventCode == EventCode_Touch)
		{
			EventTouch* et = (EventTouch*)ee;
			//for(int i=events.size()-1; i>=0; i--)
			for(std::list<EntityEvent*>::iterator it=events.begin(); it != events.end(); ++it)
			{
				if((*it)->eventCode == EventCode_Touch)
				{
					EventTouch* ettest = (EventTouch*)(*it);
					if(et->toucher == ettest->toucher)
					{ //there is already that toucher event present!
						delete et;
						return;
					}
				}
			}
		}
		events.push_back(ee);
	}
}

void Entity::switchToEditorModel()
{
	editor = true;
	if(model)
		model->deactivate();
	if(body)
		body->deactivate();
}

void Entity::switchToModel()
{
	editor = false;
	if(model)
		model->activate();
	if(body)
		body->activate();
}

void Entity::pushState(stateCallback callback, float waitTime)
{
	if(!states.empty())
	{
		states.top()->holdExecution();
	}
	states.push(new EntityState(callback, this, waitTime, 0));
	sendEvent(new EventBegin());
}

void Entity::replaceState(stateCallback callback, float waitTime)
{
	assert(!states.empty());
	states.top()->setObsolete();
	statesObsolete.push(states.top());
	//delete states.top();
	states.pop();
	pushState(callback, waitTime);
}

void Entity::popState(EntityEvent* ee)
{
	if(!states.empty())
	{
		int retIndex = states.top()->getReturnIndex();
		states.top()->setObsolete();
		statesObsolete.push(states.top());
		//delete states.top();
		states.pop();
		
		if(states.empty())
		{
			destroy();
		}

		if(retIndex != 0)
		{
			if(ee != NULL) delete ee;
			sendEvent(new EventAutowaitCallback(retIndex));
		} else
		if(ee != NULL)
		{
			sendEvent(ee);
		}
	}
}

void Entity::destroy()
{
	assert(wld);
	wld->removeEntity(this);
	return; //is that needed? x)
}

void Entity::autowait(float time, int returnIndex)
{
	assert(returnIndex != 0);
	if(!states.empty())
	{
		states.top()->holdExecution();
	}
	states.push(new EntityState(autowaitState, this, time, returnIndex));
}

void Entity::syncEntitySpeed()
{
	//sync moving and rotation speeds here!
	if(body)
	{
		body->setVelocity(desiredLinearDirection);
		body->setAngularVelocity(desiredAngularDirection);
	}
	if(model)
	{
		glm::quat q = rotateTowards(model->getRotationQuat(), glm::quat(desiredRotation), rotationSpeed*g_Delta);
		model->setRotation(glm::angle(q), glm::axis(q));
	}
}

void Entity::update()
{
	if(states.empty())
	{
		destroy();
	}
	while(!statesObsolete.empty()) { delete statesObsolete.top(); statesObsolete.pop(); }
	
	//you really think you're moving?
	syncEntitySpeed();

	//transfer pending Touch events from
	//collision callback to actual event handler
	while(!lonely())
	{
		sendEvent(new EventTouch(popToucher()));
	}
	
	states.top()->handleEvents(events);

	if(!editor)
	{
		if(model && body)
		{ //sync physical and graphical worlds :3
			model->setPosition(body->getPosition());
		}
	}
}

void Entity::initialize()
{
}

void Entity::setupModel(std::string vertShader, std::string fragShader,
						std::string modelPath, std::string diffTexture,
						std::string normTexture)
{
	assert(wldGFX);
	if(model)
		wldGFX->deleteModel(model);
	model = wldGFX->createModel(vertShader, fragShader, modelPath, diffTexture, normTexture);
}

void Entity::setupCollision(float mass)
{
	assert(wldPHY);
	if(body)
		wldPHY->remBody(body);
	body = wldPHY->addBody(mass, model); //model == NULL is handled iside the method
	body->setOwner((void*)this);
}

void Entity::setupCollision(float mass, float radius)
{
	assert(wldPHY);
	if(body)
		wldPHY->remBody(body);
	body = wldPHY->addBody(mass, radius);
	body->setOwner((void*)this);
}

void Entity::setupCollision(float mass, glm::vec3 halfExtents)
{
	assert(wldPHY);
	if(body)
		wldPHY->remBody(body);
	body = wldPHY->addBody(mass, halfExtents);
	body->setOwner((void*)this);
}

LiveEntity::LiveEntity()
{
	setClass("LiveEntity");
	health = 100.0f;
}

LiveEntity::~LiveEntity()
{
}

void LiveEntity::receiveDamage(Entity* inflictor, float damage, glm::vec3 direction)
{
}

void LiveEntity::initialize()
{
}
