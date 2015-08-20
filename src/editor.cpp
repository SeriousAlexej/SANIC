#include "editor.h"

Editor::Editor(sf::Window* w, World* wld) : p_world(wld)
{
	editorFlySpeed = 3.0f;
	edMode = Idle;
	selectedEntity = NULL;
	p_input = new InputHandler(w);
}

Editor::~Editor()
{
        delete p_input;
}

RayCastInfo Editor::castRayScreen(bool fromCenter)
{
	int screenWidth = p_input->windowSize.x, screenHeight = p_input->windowSize.y;
	sf::Vector2i mp(screenWidth/2, screenHeight/2);
	if(!fromCenter)
	{
		mp = sf::Mouse::getPosition(*p_input->mainWindow);
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

    WorldGraphics& graphics = *p_world->pGraphics;

    glm::mat4 M = glm::inverse(graphics.getCamera()->getProjectionMatrix() * graphics.getCamera()->getViewMatrix());
	glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
	glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;

    glm::vec4 lrd4(lRayEnd_world - lRayStart_world);
	glm::vec3 lRayDir_world = glm::vec3(lrd4.x, lrd4.y, lrd4.z);
	glm::vec3 lRayOrigin_world = glm::vec3(lRayStart_world.x, lRayStart_world.y, lRayStart_world.z);

	return p_world->castRay(lRayOrigin_world, lRayDir_world);
}

void Editor::updateEntity(Entity *pen)
{

}

void Editor::update()
{
	p_input->update();
    auto& physics = p_world->physics;
    auto& graphics = *p_world->pGraphics;

    physics.render(graphics.getCamera());
    graphics.render();

	if(selectedEntity != nullptr)
	{
		selectedEntity->editorUpdate();
	}
    static float doubleClickTime = g_Clock.getElapsedTime().asSeconds();
    static float moveModeTime = doubleClickTime;
    static float moveModePeriod = 1.0f;
    float tmNow = g_Clock.getElapsedTime().asSeconds();
	if(p_input->lockMouse)
	{
        edMode = Fly;
		if(p_input->wheelDelta > 0)
		{
			p_input->wheelDelta = 0;
			if(editorFlySpeed < 100.0f)
				editorFlySpeed *= 2.0f;
		} else
		if(p_input->wheelDelta < 0)
		{
			p_input->wheelDelta = 0;
			if(editorFlySpeed > 0.25f)
				editorFlySpeed *= 0.5f;
		}
		Camera* cam = graphics.getCamera();
		cam->rotate(0.1f * g_Delta * p_input->mouseDelta.x,
					0.1f * g_Delta * p_input->mouseDelta.y);
        if(p_input->keyPressed(sf::Keyboard::Up))
		{
			cam->moveFront(editorFlySpeed*g_Delta);
		} else
        if(p_input->keyPressed(sf::Keyboard::Down))
		{
			cam->moveFront(-editorFlySpeed*g_Delta);
		}
        if(p_input->keyPressed(sf::Keyboard::Right))
		{
			cam->moveRight(editorFlySpeed*g_Delta);
		} else
        if(p_input->keyPressed(sf::Keyboard::Left))
		{
			cam->moveRight(-editorFlySpeed*g_Delta);
		}
		if(p_input->keyPressed(sf::Keyboard::Space))
		{
			cam->moveUp(editorFlySpeed*g_Delta);
		} else
		if(p_input->keyPressed(sf::Keyboard::C))
		{
			cam->moveUp(-editorFlySpeed*g_Delta);
		}
	} else {
        if(edMode == Fly)
        {
            edMode = Idle;
        }
		if(p_input->cursorIsInsideWindow())
		{

        if(edMode==Idle && p_input->keyJustReleased(sf::Keyboard::Delete))
        {
            if(selectedEntity != NULL)
            {
                selectedEntity->editorDesselect();
                p_world->removeEntity(selectedEntity);
                selectedEntity = NULL;
                return;
            }
        }

        if(edMode==Idle && p_input->keyJustReleased(sf::Keyboard::Q))
        {
            if(selectedEntity != NULL)
            {
                selectedEntity->editorDesselect();
                selectedEntity = NULL;
                return;
            }
        }

        if(selectedEntity != NULL && edMode == Idle && p_input->keyJustReleased(sf::Keyboard::R))
        {
            RayCastInfo ri = castRayScreen();
            EntityPointer* ep = selectedEntity->getTargetPointer();
            if(ep != nullptr)
            {
                if(ep->Name() == "Parent")
                {
                    selectedEntity->setParent(ri.enHit);
                } else {
                    (*ep) = ri.enHit;
                }
                selectedEntity->pointerIndexPrevious = -1; //update target info string
            }
        }

		if(selectedEntity != NULL && edMode == Idle && p_input->keyPressed(sf::Keyboard::LControl) && p_input->mouseButtonJustPressed(sf::Mouse::Right))
		{
		    edMode = Pulling;
            sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow);
            mposOffsetMoving = glm::vec2(mpos.x, mpos.y);
		}

		if(edMode == Pulling)
        {
            if(selectedEntity == NULL) { edMode = Idle; return; }
            sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow);
            glm::vec3 camPos = graphics.getCamera()->getPosition();
            camPos = camPos - selectedEntity->position;
            camPos *= editorFlySpeed*((mpos.y - mposOffsetMoving.y)/(1.0f*p_input->windowSize.y))/glm::length(camPos);
            selectedEntity->position += camPos;
            mposOffsetMoving = glm::vec2(mpos.x, mpos.y);
        }

		if(selectedEntity != NULL && edMode == Idle && p_input->keyPressed(sf::Keyboard::LControl) && p_input->mouseButtonJustPressed(sf::Mouse::Left))
		{
		    if(tmNow-doubleClickTime<=0.5f && (moveModePeriod<0.1f || tmNow-moveModeTime-moveModePeriod>=1.0f))
            {
                RayCastInfo ri = castRayScreen();
                if(ri.enHit!=NULL)
                {
                    selectedEntity->position = ri.posHit;
                } else {
                    selectedEntity->position = ri.posOrigin+ri.direction*5.0f;
                }
                doubleClickTime -= 10.0f;
                return;
            }
		    doubleClickTime = tmNow;

            edMode = Moving;
            moveModeTime = tmNow;
            sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow);
            mpos.y = p_input->windowSize.y - mpos.y;
            glm::mat4 camMat = graphics.getCamera()->getProjectionMatrix() * graphics.getCamera()->getViewMatrix();
            glm::vec4 entPos = glm::vec4(selectedEntity->position.x, selectedEntity->position.y, selectedEntity->position.z, 1.0f);
            entPos =  camMat * entPos; entPos/=entPos.w;
            mposOffsetMoving = glm::vec2(entPos.x - (float(mpos.x)/float(p_input->windowSize.x))*2.0f + 1.0f,
                                         entPos.y - (float(mpos.y)/float(p_input->windowSize.y))*2.0f + 1.0f);
		}

		if(edMode == Moving)
		{
            if(selectedEntity == NULL) { edMode = Idle; return; }
            glm::mat4 camMat = graphics.getCamera()->getProjectionMatrix() * graphics.getCamera()->getViewMatrix();
            glm::vec4 entPos = glm::vec4(selectedEntity->position.x, selectedEntity->position.y, selectedEntity->position.z, 1.0f);
            entPos =  camMat * entPos;
            sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow);
            mpos.y = p_input->windowSize.y - mpos.y;
            glm::vec4 endPosNDC = glm::vec4( (float(mpos.x)/float(p_input->windowSize.x))*2.0f - 1.0f + mposOffsetMoving.x,
                                             (float(mpos.y)/float(p_input->windowSize.y))*2.0f - 1.0f + mposOffsetMoving.y,
                                             entPos.z/entPos.w,
                                             1.0f);
            glm::vec4 endPosWLD = glm::inverse(camMat) * endPosNDC; endPosWLD/=endPosWLD.w;
            selectedEntity->position = glm::vec3(endPosWLD.x, endPosWLD.y, endPosWLD.z);
		}


		if(p_input->mouseButtonJustReleased(sf::Mouse::Right))
		{
            if(edMode != Idle) { edMode = Idle; return; }
		}

		if(p_input->mouseButtonJustReleased(sf::Mouse::Left))
		{
            if(edMode != Idle)
            {
                if(edMode == Moving)
                {
                    moveModePeriod = tmNow - moveModeTime;
                }
                edMode = Idle;
                return;
            }

            if(tmNow-doubleClickTime<0.5f) return;
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
		}
	}
}
