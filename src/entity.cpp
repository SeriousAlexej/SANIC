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

void TW_CALL clearPointer(void *boolPtr)
{
    (*(bool*)boolPtr) = true;
}

Entity::Entity()
{
	wld = NULL;
	wldGFX = NULL;
	wldPHY = NULL;
	editor = false;
	entityBar = NULL;
	orientationType = BY_BODY;
	translatedByBody = true;
	body = NULL;
	model = NULL;
	pushState(dummy, 0);
	setClass("Entity");
	desiredLinearDirection = glm::vec3(0,0,0);
	desiredAngularDirection = glm::vec3(0,0,0);
	desiredRotation = glm::vec3(0,0,0);
	rotationSpeed = 1.0f;
	rotationQuat = glm::quat();
	rotationEuler = glm::vec3(0,0,0);
	rotationQuatO = glm::quat();
	rotationEulerO = glm::vec3(0,0,0);
	position = glm::vec3(0,0,0);
	parent = NULL;
	relativeTransform = glm::mat4(1);
	oldMatrix = glm::mat4(1);
	pointerIndex = 0;
	pointerIndexPrevious = 0;
	shouldClearPointer = false;

    registerProperties(
                "Name",     &name,
                "Quat",     &rotationQuat,
                "RotH",     &rotationEuler[1],
                "RotP",     &rotationEuler[0],
                "RotB",     &rotationEuler[2],
                "PosX",     &position[0],
                "PosY",     &position[1],
                "PosZ",     &position[2]
    );

    addDrawableElements(
        {
            {
                DrawableElement{DrawableElement::PT_STRING, "Name", "label='Name' "}
            },
            {
                DrawableElement{DrawableElement::PT_QUAT, "Quat", "label='Quaternion' opened=true "},
                DrawableElement{DrawableElement::PT_FLOAT, "RotH", "label='H (Y axis)' precision=2 step=0.25 "},
                DrawableElement{DrawableElement::PT_FLOAT, "RotP", "label='P (X axis)' precision=2 step=0.25 "},
                DrawableElement{DrawableElement::PT_FLOAT, "RotB", "label='B (Z axis)' precision=2 step=0.25 "}
            },
            {
                DrawableElement{DrawableElement::PT_FLOAT, "PosX", "label='X' precision=2 step=0.01 "},
                DrawableElement{DrawableElement::PT_FLOAT, "PosY", "label='Y' precision=2 step=0.01 "},
                DrawableElement{DrawableElement::PT_FLOAT, "PosZ", "label='Z' precision=2 step=0.01 "}
            },
            {
                DrawableElement{DrawableElement::PT_ENUM, "Pointer", "", &pointerIndex, NULL, this->getPointersString()},
                DrawableElement{DrawableElement::PT_BUTTON, "PointerValue", "label='"+getPointerDescr()+"'", NULL, NULL},
                DrawableElement{DrawableElement::PT_BUTTON, "Clear", "", &shouldClearPointer, clearPointer}
            }
        }
    );
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

	for(int i=children.size()-1; i>=0; i--)
	{
        children[i]->parent = NULL;
	}
	children.clear();

	if(parent)
    for(int i=parent->children.size()-1; i>=0; i--)
    {
        if(parent->children[i] == this)
        {
            parent->children.erase(parent->children.begin() + i);
            break;
        }
    }

	while(!statesObsolete.empty()) { delete statesObsolete.top(); statesObsolete.pop(); }
	while(!states.empty()) { delete states.top(); states.pop(); }
	while(!events.empty()) { delete events.front(); events.pop_front(); }
}

std::string Entity::getName()  {
    return properties["Name"]->GetValue<std::string>();
}

void Entity::setName(string newName) {
    properties["Name"]->SetValue<std::string>(newName);
}

template<class T>
T& Entity::getProperty(std::string name)
{
    return properties[name]->GetValue<T>();
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
	if(g_Editor)
	{
		editor = true;
		if(model)
			model->deactivate();
		if(body)
			body->deactivate();
	}
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
		if(translatedByBody)
			body->setVelocity(desiredLinearDirection);
		body->setAngularVelocity(desiredAngularDirection);
	}
	if(model && orientationType != NONE)
	{
		glm::quat q;
		if(body && orientationType == BY_BODY)
		{
			q = body->getRotationQuat();
		} else {
			q = rotateTowards(model->getRotationQuat(), glm::quat(desiredRotation), rotationSpeed*g_Delta);
		}
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
	adjustMoving();

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
	updatePosition();
}

void Entity::renderSelectionIndicator()
{
    if(!model) return;
	glUseProgram(0);
	glm::mat4 proj = wldGFX->getCamera()->getProjectionMatrix();
	glm::mat4 view = wldGFX->getCamera()->getViewMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(&proj[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(&view[0][0]);
	glPushMatrix();
	glm::vec3 spherePos = model->getRenBoxCenter();
	spherePos.y += model->getRenBoxHalfSizes().y + model->getRenSphere().w*0.125;
	glTranslatef(spherePos.x, spherePos.y, spherePos.z);
	glRotatef(-90.0f, 1,0,0);
	glColor3f(1.0f, 0.0f, 0.0f);
	glDisable(GL_CULL_FACE);
	GLUquadricObj* quadric = gluNewQuadric();
	gluCylinder(quadric, 0, model->getRenSphere().w*0.125f, model->getRenSphere().w*0.25f, 8, 1);
	glTranslatef(0,0,model->getRenSphere().w*0.25f);
	gluCylinder(quadric, model->getRenSphere().w*0.04f, model->getRenSphere().w*0.04f, model->getRenSphere().w*0.25f, 8, 1);
	gluDeleteQuadric(quadric);
	glEnable(GL_CULL_FACE);
	glPopMatrix();
}

void Entity::parentMoved()
{
    glm::mat4 newPos = glm::transpose(relativeTransform * glm::transpose(parent->model->getMatrix()));
    position = glm::vec3(newPos[3][0],newPos[3][1],newPos[3][2]);
    rotationQuat = glm::quat(newPos);
    if(model)
    {
        model->setPosition(position);
        model->setRotation(glm::angle(rotationQuat), glm::axis(rotationQuat));
    }
    if(body)
    {
        body->setPosition(position);
        body->setRotation(glm::angle(rotationQuat), glm::axis(rotationQuat));
    }
    oldMatrix = model->getMatrix();
    for(int i=children.size()-1; i>=0; i--)
    {
        children[i]->parentMoved();
    }
    adjustMoving();
}

void Entity::updatePosition()
{
	if(model)
	{
        glm::mat4 newMatrix = model->getMatrix();
        if(newMatrix != oldMatrix)
        {
            if(parent)
            {
                relativeTransform = glm::transpose(model->getMatrix()) * glm::inverse(glm::transpose(parent->model->getMatrix()));
            }
            for(int i=children.size()-1; i>=0; i--)
            {
                children[i]->parentMoved();
            }
            oldMatrix = newMatrix;
        }
    }
}

bool Entity::childrenContain(Entity* e) const
{
    for(int i=children.size()-1; i>=0; i--)
    {
        if(children[i]==e || children[i]->childrenContain(e))
            return true;
    }
    return false;
}

void Entity::setParent(Entity* p)
{
    if(p == this || p == parent || childrenContain(p)) { return; }
    if(parent)
    for(int i=parent->children.size()-1; i>=0; i--)
    {
        if(parent->children[i] == this)
        {
            parent->children.erase(parent->children.begin() + i);
            break;
        }
    }
    parent = p;
    if(parent)
    {
        parent->children.push_back(this);
        relativeTransform = glm::transpose(model->getMatrix()) * glm::inverse(glm::transpose(parent->model->getMatrix()));
    }
}

Entity* Entity::getParent() const
{
    return parent;
}

void Entity::editorUpdate()
{
    if(rotationEuler != rotationEulerO)
    {//update quaternion
        for(int i=0; i<3; i++)
        {
            while(rotationEuler[i] > 180.0f) rotationEuler[i] -= 360.0f;
            while(rotationEuler[i] <-180.0f) rotationEuler[i] += 360.0f;
        }
        rotationQuat = glm::quat(glm::vec3(glm::radians(rotationEuler.x),
                                           glm::radians(rotationEuler.y),
                                           glm::radians(rotationEuler.z)));
    } else
    if(rotationQuat != rotationQuatO)
    {//update euler
        rotationEuler = glm::eulerAngles(rotationQuat);
    }
    rotationEulerO = rotationEuler;
    rotationQuatO = rotationQuat;

    body->setRotation(glm::angle(rotationQuat), glm::axis(rotationQuat));
    model->setRotation(glm::angle(rotationQuat), glm::axis(rotationQuat));

    model->setPosition(position);
    body->setPosition(position);

    char idStr[15];
    sprintf(idStr, "(ID=%d)", getMultipass());
    std::string barLabel = (getClass()+"::"+getName()+idStr);
    TwDefine((" EntityBar label='"+barLabel+"' ").c_str());

    if(shouldClearPointer)
    {
        shouldClearPointer = false;
        Entity** tp = getTargetPointer();
        if(tp)
        {
            if(tp==&parent)
            {
                setParent(NULL);
            } else {
                *tp = NULL;
            }
            pointerIndexPrevious = -1;
        }
    }

    if(pointerIndex != pointerIndexPrevious)
    {
        TwDefine(("EntityBar/PointerValue label='"+getPointerDescr()+"'").c_str());
        pointerIndexPrevious = pointerIndex;
    }

    updatePosition();
	adjustMoving();
	renderSelectionIndicator();
}

void Entity::addDrawableElements(initializer_list<initializer_list<DrawableElement>> lle)
{
    static unsigned int groupID = 0;
    string groupName = " group=g";
    for(auto le : lle)
    {
        int sz = guiElements.size();
        guiElements.push_back(vector<DrawableElement>());
        for(auto e : le)
        {
            DrawableElement em = e;
            em.drawingHint += groupName;
            guiElements[sz].push_back(e);
        }
        groupName += to_string(++groupID);
    }
}

void Entity::drawSingleElement(DrawableElement &elem)
{
    switch(elem.tp)
    {
        case TW_TYPE_QUAT4F :
        {
            TwAddVarRW(entityBar, elem.name.c_str(), TW_TYPE_QUAT4F, properties[elem.name]->m_data, elem.drawingHint.c_str());
            break;
        }
        case TW_TYPE_STDSTRING :
        {
            TwAddVarRW(entityBar, elem.name.c_str(), TW_TYPE_STDSTRING, properties[elem.name]->m_data, elem.drawingHint.c_str());
            break;
        }
        case TW_TYPE_FLOAT :
        {
            TwAddVarRW(entityBar, elem.name.c_str(), TW_TYPE_FLOAT, properties[elem.name]->m_data, elem.drawingHint.c_str());
            break;
        }
        case DrawableElement::PT_ENUM :
        {
            TwType pointersType = TwDefineEnumFromString((elem.name+"Types").c_str(), elem.enumTypes.c_str());
            TwAddVarRW(entityBar, elem.name.c_str(), pointersType, elem.clientVar, elem.drawingHint.c_str());
            break;
        }
        case DrawableElement::PT_BUTTON :
        {
            TwAddButton(entityBar, elem.name.c_str(), (elem.buttonCallback!=NULL?*elem.buttonCallback.target<void(*)(void*)>():NULL), elem.clientVar, elem.drawingHint.c_str());
            break;
        }
        default : { printf("ERROR: Unknown gui element type\n"); }
    }
}

void Entity::drawGuiElements()
{
    static unsigned int sepID = 0;
    std::string sepname = "s";
    for(std::vector<DrawableElement> &v : guiElements)
    {
        for(DrawableElement &d : v)
        {
            drawSingleElement(d);
        }
        TwAddSeparator(entityBar, sepname.c_str(), "");
        sepname += std::to_string(++sepID);
    }
}

void Entity::editorSelect()
{
    assert(entityBar == NULL);
    entityBar = TwNewBar("EntityBar");

    char idStr[15]; //example "Model::Mountain(ID=42)"
    sprintf(idStr, "(ID=%d)", getMultipass());
    std::string barLabel = (getClass()+"::"+getName()+idStr);
    TwDefine((" EntityBar label='"+barLabel+"' fontSize=3 ").c_str());

    drawGuiElements();

    pointerIndexPrevious = -1;
}

Entity** Entity::getTargetPointer()
{
    switch(pointerIndex)
    {
        case 0: return &parent; break;
        default: return NULL;
    }
}

std::string Entity::getPointerDescr()
{
    Entity** e = getTargetPointer();
    if(e && *e)
    {
        char idStr[15];
        sprintf(idStr, "(ID=%d)", (*e)->getMultipass());
        return (*e)->getName()+idStr;
    }
    return "(none)";
}

std::string Entity::getPointersString()
{
    return "Parent";
}

void Entity::editorDesselect()
{
    assert(TwDeleteBar(entityBar)==1);
    entityBar = NULL;
}

void Entity::adjustMoving()
{
}

void Entity::initialize()
{
}

void Entity::setupModel(std::string shaderPath,
						std::string modelPath, std::string diffTexture,
						std::string normTexture, std::string heightTexture)
{
	assert(wldGFX);
	if(model)
		wldGFX->deleteModel(model);
	model = wldGFX->createModel(shaderPath, modelPath, diffTexture, normTexture, heightTexture);
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
	maxHealth = 100.0f;
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
