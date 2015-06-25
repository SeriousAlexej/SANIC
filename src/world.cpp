#include "world.h"
#include "entities/incubator.h"

World::World(sf::Window* w)
{
	editorFlySpeed = 3.0f;
	input = new InputHandler(w);
	edMode = Idle;
	selectedEntity = NULL;

	if(g_Editor)
	{
		//entitiesList = TwNewBar("EntitiesList");
		//TwDefine(" EntitiesList label='Entities' fontSize=3 position='0 200' size='100 568'");
	}
}

World::~World()
{
	for(int i=entities.size()-1; i>=0; i--)
	{
		//delete entities[i];
        void* ptr = (dynamic_cast<FromIncubator*>(entities[i]))->ptr;
        entities[i]->~Entity();
        ::operator delete(ptr);
	}
	entities.clear();
	Incubator::deleteInstance();
	delete input;
}

void World::update()
{
	input->update();
	if(!g_Editor)
	{
		for(int i=entities.size()-1; i>=0; i--)
		{
			entities[i]->update();
		}
		physics.update();
	}
	graphics.render();
	if(g_Editor)
	{
		physics.render(graphics.getCamera());
		updateEditor();
		if(selectedEntity != nullptr)
		{
            selectedEntity->editorUpdate();
		}
	}
}

void World::updateEditor()
{
    static float doubleClickTime = g_Clock.getElapsedTime().asSeconds();
	if(input->lockMouse)
	{
        edMode = Fly;
		if(input->wheelDelta > 0)
		{
			input->wheelDelta = 0;
			if(editorFlySpeed < 100.0f)
				editorFlySpeed *= 2.0f;
		} else
		if(input->wheelDelta < 0)
		{
			input->wheelDelta = 0;
			if(editorFlySpeed > 0.25f)
				editorFlySpeed *= 0.5f;
		}
		Camera* cam = graphics.getCamera();
		cam->rotate(0.1f * g_Delta * input->mouseDelta.x,
					0.1f * g_Delta * input->mouseDelta.y);
		if(input->keyPressed(sf::Keyboard::W))
		{
			cam->moveFront(editorFlySpeed*g_Delta);
		} else
		if(input->keyPressed(sf::Keyboard::S))
		{
			cam->moveFront(-editorFlySpeed*g_Delta);
		}
		if(input->keyPressed(sf::Keyboard::D))
		{
			cam->moveRight(editorFlySpeed*g_Delta);
		} else
		if(input->keyPressed(sf::Keyboard::A))
		{
			cam->moveRight(-editorFlySpeed*g_Delta);
		}
		if(input->keyPressed(sf::Keyboard::Space))
		{
			cam->moveUp(editorFlySpeed*g_Delta);
		} else
		if(input->keyPressed(sf::Keyboard::C))
		{
			cam->moveUp(-editorFlySpeed*g_Delta);
		}
	} else {
        if(edMode == Fly)
        {
            edMode = Idle;
        }
		if(input->cursorIsInsideWindow())
		{

        if(edMode==Idle && input->keyJustReleased(sf::Keyboard::Delete))
        {
            if(selectedEntity != NULL)
            {
                selectedEntity->editorDesselect();
                removeEntity(selectedEntity);
                selectedEntity = NULL;
                return;
            }
        }

        if(edMode==Idle && input->keyJustReleased(sf::Keyboard::Q))
        {
            if(selectedEntity != NULL)
            {
                selectedEntity->editorDesselect();
                selectedEntity = NULL;
                return;
            }
        }

        if(selectedEntity != NULL && edMode == Idle && input->keyJustReleased(sf::Keyboard::R))
        {
            RayCastInfo ri = castRayScreen();
            EntityPointer* ep = selectedEntity->getTargetPointer();
            if(ep != nullptr)
            {
                if(ep->Name() == "Parent")
                {
                    selectedEntity->setParent(ri.enHit);
                }
                (*ep) = ri.enHit;
                selectedEntity->pointerIndexPrevious = -1; //update target info string
            }
        }

		if(selectedEntity != NULL && edMode == Idle && input->keyPressed(sf::Keyboard::LControl) && input->mouseButtonJustPressed(sf::Mouse::Right))
		{
		    edMode = Pulling;
            sf::Vector2i mpos = sf::Mouse::getPosition(*input->mainWindow);
            mposOffsetMoving = glm::vec2(mpos.x, mpos.y);
		}

		if(edMode == Pulling)
        {
            if(selectedEntity == NULL) { edMode = Idle; return; }
            sf::Vector2i mpos = sf::Mouse::getPosition(*input->mainWindow);
            glm::vec3 camPos = graphics.getCamera()->getPosition();
            camPos = camPos - selectedEntity->position;
            camPos *= editorFlySpeed*((mpos.y - mposOffsetMoving.y)/(1.0f*input->windowSize.y))/glm::length(camPos);
            selectedEntity->position += camPos;
            mposOffsetMoving = glm::vec2(mpos.x, mpos.y);
        }

		if(selectedEntity != NULL && edMode == Idle && input->keyPressed(sf::Keyboard::LControl) && input->mouseButtonJustPressed(sf::Mouse::Left))
		{
		    float tmNow = g_Clock.getElapsedTime().asSeconds();
		    if(tmNow-doubleClickTime<=0.5f)
            {
                RayCastInfo ri = castRayScreen();
                if(ri.enHit!=NULL)
                {
                    selectedEntity->position = ri.posHit;
                } else {
                    selectedEntity->position = ri.posOrigin+ri.direction*5.0f;
                }
                return;
            }
		    doubleClickTime = tmNow;

            edMode = Moving;
            sf::Vector2i mpos = sf::Mouse::getPosition(*input->mainWindow);
            mpos.y = input->windowSize.y - mpos.y;
            glm::mat4 camMat = graphics.getCamera()->getProjectionMatrix() * graphics.getCamera()->getViewMatrix();
            glm::vec4 entPos = glm::vec4(selectedEntity->position.x, selectedEntity->position.y, selectedEntity->position.z, 1.0f);
            entPos =  camMat * entPos; entPos/=entPos.w;
            mposOffsetMoving = glm::vec2(entPos.x - (float(mpos.x)/float(input->windowSize.x))*2.0f + 1.0f,
                                         entPos.y - (float(mpos.y)/float(input->windowSize.y))*2.0f + 1.0f);
		}

		if(edMode == Moving)
		{
            if(selectedEntity == NULL) { edMode = Idle; return; }
            glm::mat4 camMat = graphics.getCamera()->getProjectionMatrix() * graphics.getCamera()->getViewMatrix();
            glm::vec4 entPos = glm::vec4(selectedEntity->position.x, selectedEntity->position.y, selectedEntity->position.z, 1.0f);
            entPos =  camMat * entPos;
            sf::Vector2i mpos = sf::Mouse::getPosition(*input->mainWindow);
            mpos.y = input->windowSize.y - mpos.y;
            glm::vec4 endPosNDC = glm::vec4( (float(mpos.x)/float(input->windowSize.x))*2.0f - 1.0f + mposOffsetMoving.x,
                                             (float(mpos.y)/float(input->windowSize.y))*2.0f - 1.0f + mposOffsetMoving.y,
                                             entPos.z/entPos.w,
                                             1.0f);
            glm::vec4 endPosWLD = glm::inverse(camMat) * endPosNDC; endPosWLD/=endPosWLD.w;
            selectedEntity->position = glm::vec3(endPosWLD.x, endPosWLD.y, endPosWLD.z);
		}

		if(input->mouseButtonJustReleased(sf::Mouse::Right))
		{
            if(edMode != Idle) { edMode = Idle; return; }
		}

		if(input->mouseButtonJustReleased(sf::Mouse::Left))
		{
            if(edMode != Idle) { edMode = Idle; return; }

            if(g_Clock.getElapsedTime().asSeconds()-doubleClickTime<0.5f) return;
            RayCastInfo rci = castRayScreen();
            if(rci.enHit != NULL)
            {
                if(selectedEntity != NULL)
                {
                    if(selectedEntity == rci.enHit) return;
                    selectedEntity->editorDesselect();
                }
                selectedEntity = rci.enHit;
                selectedEntity->editorSelect();
            } else {
                if(selectedEntity != NULL)
                {
                    selectedEntity->editorDesselect();
                    selectedEntity = NULL;
                }
            }
		}
		} else {
            if(edMode != Idle)
                edMode = Idle;
		}
	}
}

Entity* World::createEntity(std::string entityName)
{
    return createEntity(static_cast<Entity*>(Incubator::Create(entityName)));
}

Entity* World::createEntity(Entity* e)
{
	//can't add already added or null entity
	assert(e != nullptr && e->wldGFX == nullptr && e->wldPHY == nullptr && e->wld == nullptr);
	e->wldGFX = &graphics;
	e->wldPHY = &physics;
	e->wld = this;
	e->initialize();
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
				//delete e;
				void* ptr = (dynamic_cast<FromIncubator*>(e))->ptr;
				e->~Entity();
				::operator delete(ptr);
				entities.erase(entities.begin() + i);
				return;
			}
		}
	}
}

RayCastInfo World::castRay(glm::vec3 origin, glm::vec3 direction)
{
	RayCastInfo rci;
	rci.enHit = NULL;
	rci.normHit = glm::vec3(0,0,0);
	rci.posHit = glm::vec3(0,0,0);
	rci.posOrigin = origin;
	rci.direction = glm::normalize(direction);
	rci.rayLength = 0.0f;

	direction = glm::normalize(direction)*1000.0f;

	btCollisionWorld::ClosestRayResultCallback RayCallback(
		btVector3(origin.x, origin.y, origin.z),
		btVector3(direction.x, direction.y, direction.z)
		);
	physics.dynamicsWorld->rayTest(
		btVector3(origin.x, origin.y, origin.z),
		btVector3(direction.x, direction.y, direction.z),
		RayCallback
		);

	if(RayCallback.hasHit() && RayCallback.m_collisionObject->getUserPointer() != nullptr)
	{
		rci.enHit = static_cast<Entity*>(RayCallback.m_collisionObject->getUserPointer());
		btVector3 &hw = RayCallback.m_hitPointWorld;
		btVector3 &nw = RayCallback.m_hitNormalWorld;
		rci.normHit = glm::vec3(nw.getX(), nw.getY(), nw.getZ());
		rci.posHit = glm::vec3(hw.getX(), hw.getY(), hw.getZ());
		rci.rayLength = glm::distance(rci.posHit, rci.posOrigin);
	}

	return rci;
}

RayCastInfo World::castRayScreen(bool fromCenter)
{
	int screenWidth = input->windowSize.x, screenHeight = input->windowSize.y;
	sf::Vector2i mp(screenWidth/2, screenHeight/2);
	if(!fromCenter)
	{
		mp = sf::Mouse::getPosition(*input->mainWindow);
		mp.y = screenHeight - mp.y;
    }

	int mouseX = mp.x, mouseY = mp.y;

	glm::vec4 lRayStart_NDC(
		(float(mouseX)/float(screenWidth)  - 0.5f) * 2.0f,
		(float(mouseY)/float(screenHeight) - 0.5f) * 2.0f,
		-1.0,
		1.0f
		);
	glm::vec4 lRayEnd_NDC(
		(float(mouseX)/float(screenWidth)  - 0.5f) * 2.0f,
		(float(mouseY)/float(screenHeight) - 0.5f) * 2.0f,
		1.0,
		1.0f
		);

	glm::mat4 M = glm::inverse(graphics.getCamera()->getProjectionMatrix() * graphics.getCamera()->getViewMatrix());
	glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
	glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;

    glm::vec4 lrd4(lRayEnd_world - lRayStart_world);
	glm::vec3 lRayDir_world = glm::vec3(lrd4.x, lrd4.y, lrd4.z);
	glm::vec3 lRayOrigin_world = glm::vec3(lRayStart_world.x, lRayStart_world.y, lRayStart_world.z);

	return castRay(lRayOrigin_world, lRayDir_world);
}
