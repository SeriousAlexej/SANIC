#include "entity.h"
#include "quaternion_utils.h"
#include "editorGUI.h"
#include "global.h"
#include <rapidjson/writer.h>

extern std::map<int, Entity*> enByOldId;

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
			EventAutowaitCallback* eac = dynamic_cast<EventAutowaitCallback*>(ee);
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
    (*static_cast<bool*>(boolPtr)) = true;
}

void Entity::registerPointers()
{
    unsigned i;
    for(i=0; i<pointers.size()-1; i++)
    {
        pointersString += pointers[i].Name() + ",";
        registerProperties(pointers[i].Name(), &pointers[i]);
    }
    pointersString += pointers[i].Name();
    registerProperties(pointers[i].Name(), &pointers[i]);
}

void Entity::fillPointers()
{
    pointers.push_back(EntityPointer("Parent"));
}

Entity::Entity()
{
    pointersString = "";
	wld = nullptr;
	wldGFX = nullptr;
	wldPHY = nullptr;
	editor = false;
	entityBar = nullptr;
	body = nullptr;
	model = nullptr;
	pushState(dummy, 0);
	setClass("Entity");
	rotationQuat = glm::quat();
	rotationEuler = glm::vec3(0,0,0);
	rotationQuatO = glm::quat();
	rotationEulerO = glm::vec3(0,0,0);
	position = glm::vec3(0,0,0);
	parent = nullptr;
	relativeTransform = glm::mat4(1);
	oldMatrix = glm::mat4(1);
	pointerIndex = 0;
	pointerIndexPrevious = 0;
	shouldClearPointer = false;
    fillPointers();
}

void Entity::pointerLeft(EntityPointer* pen)
{
    for(int i=pointAtMe.size()-1; i>=0; i--)
    {
        if(pointAtMe[i]==pen)
        {
            pointAtMe.erase(pointAtMe.begin()+i);
            break;
        }
    }
}

void Entity::pointerAdded(EntityPointer* pen)
{
    assert(pen != nullptr);
    pointAtMe.push_back(pen);
}

void Entity::registerLua(LuaUserdata<Entity>& lua) // TODO: Make it more "generic"
{
    addToLua(lua,
            "Name",    name,
            //"Quat",    rotationQuat,
            "RotH",    rotationEuler[1],
            "RotP",    rotationEuler[0],
            "RotB",    rotationEuler[2],
            "PosX",    position[0],
            "PosY",    position[1],
            "PosZ",    position[2],
            "Parent",  pointers[0]
    );
}

void Entity::addProperties()
{
	registerPointers();

    registerProperties(
                "Name",     &name,
                "Quat",     &rotationQuat,
                "RotH",     &rotationEuler[1],
                "RotP",     &rotationEuler[0],
                "RotB",     &rotationEuler[2],
                "PosX",     &position[0],
                "PosY",     &position[1],
                "PosZ",     &position[2],
                "Editor",   &editor
    );

    addDrawableElements(
        {
            {
                DrawableElement{DrawableElement::PT_STRING, "Name", "label='Name' "},
                DrawableElement{DrawableElement::PT_BOOL, "Editor", "label='Editor Only'"}
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
                DrawableElement{DrawableElement::PT_ENUM, "Pointer", "", &pointerIndex, NULL, pointersString},
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
        children[i]->parent = nullptr;
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

    for(auto &p : properties)
    {
        delete p.second;
    }

    for(auto &p : pointAtMe)
    {
        (*p) = nullptr;
    }

    pointers.clear();

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

template<class T>
void Entity::setProperty(string s, T& val)
{
    properties[s]->SetValue<T>(val);
}

void Entity::sendEvent(EntityEvent *ee)
{
	if(ee)
	{
		//they say Bullet sends multiple CollisionCallbacks...
		//let's fix this by deleting duplicate touch events!
		if(ee->eventCode == EventCode_Touch)
		{
			EventTouch* et = dynamic_cast<EventTouch*>(ee);
			//for(int i=events.size()-1; i>=0; i--)
			for(std::list<EntityEvent*>::iterator it=events.begin(); it != events.end(); ++it)
			{
				if((*it)->eventCode == EventCode_Touch)
				{
					EventTouch* ettest = dynamic_cast<EventTouch*>(*it);
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
    if(!egg::getInstance().g_Editor)
	{
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

void Entity::syncEntityRotation()
{
    if(body && model)
    {
        model->setRotation(body->getRotationQuat());
    }
    /*
	//sync moving and rotation speeds here!
	if(body)
	{
		//if(translatedByBody)
		//	body->setVelocity(desiredLinearDirection);
		//body->setAngularVelocity(desiredAngularDirection);
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
	*/
}

void Entity::update()
{
	if(states.empty())
	{
		destroy();
	}
	while(!statesObsolete.empty()) { delete statesObsolete.top(); statesObsolete.pop(); }

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
	//sync rotation
	syncEntityRotation();
	updatePosition();
	adjustMoving();
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
    setRotation(rotationQuat);
    setPosition(position);
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
    for(auto &pt : pointers)
    {
        if(pt.Name() == "Parent")
        {
            pt = p;
            break;
        }
    }
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
        rotationEuler = glm::eulerAngles(rotationQuat)*glm::one_over_pi<float>()*180.0f;
    }
    rotationEulerO = rotationEuler;
    rotationQuatO = rotationQuat;

    setRotation(rotationQuat);
    setPosition(position);

    char idStr[15];
    sprintf(idStr, "(ID=%d)", getMultipass());
    std::string barLabel = (getClass()+"::"+getName()+idStr);
    TwDefine((" EntityBar label='"+barLabel+"' ").c_str());

    if(shouldClearPointer)
    {
        shouldClearPointer = false;
        EntityPointer* tp = getTargetPointer();
        if(tp != nullptr)
        {
            if(tp->Name()=="Parent")
            {
                setParent(nullptr);
            }
            (*tp) = nullptr;
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
    for(auto le : lle)
    {
        int sz = guiElements.size();
        guiElements.push_back(vector<DrawableElement>());
        for(auto e : le)
        {
            guiElements[sz].push_back(e);
        }
    }
}

void Entity::drawSingleElement(DrawableElement &elem)
{
    switch(elem.tp)
    {
        case TW_TYPE_BOOLCPP :
        {
            TwAddVarRW(entityBar, elem.name.c_str(), TW_TYPE_BOOLCPP, properties[elem.name]->m_data, elem.drawingHint.c_str());
            break;
        }
        case TW_TYPE_COLOR3F :
        {
            TwAddVarRW(entityBar, elem.name.c_str(), TW_TYPE_COLOR3F, properties[elem.name]->m_data, elem.drawingHint.c_str());
            break;
        }
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
    TwRemoveAllVars(entityBar);
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
    std::string barPosition = "position='"+std::to_string(leftWndWidth)+" "+std::to_string(topWndHeight)+"' ";
    std::string barSize = "size='"+std::to_string(barWidth)+" "+std::to_string(egg::getInstance().g_Resolution.y)+"' ";
    TwDefine((" EntityBar label='"+barLabel+"' color='70 70 70' alpha=200 valueswidth="+std::to_string(barWidth/2)+" fontSize=2 resizable=false movable=false iconifiable=false " + barPosition + barSize).c_str());

    drawGuiElements();

    pointerIndexPrevious = -1;
}

EntityPointer* Entity::getTargetPointer()
{
    if(pointerIndex>=0 && static_cast<unsigned>(pointerIndex)<pointers.size())
    {
        return &pointers[pointerIndex];
    }
    return nullptr;
}

std::string Entity::getPointerDescr()
{
    EntityPointer* ptr = getTargetPointer();
    if(ptr != nullptr)
    {
        EntityPointer& p = *ptr;
        if(p)
        {
            char idStr[15];
            sprintf(idStr, "(ID=%d)", p->getMultipass());
            return p->getName()+idStr;
        }
    }
    return "(none)";
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
    addProperties();
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
	body->setOwner(static_cast<void*>(this));
}

void Entity::setupCollision(float mass, float radius)
{
	assert(wldPHY);
	if(body)
		wldPHY->remBody(body);
	body = wldPHY->addBody(mass, radius);
	body->setOwner(static_cast<void*>(this));
}

void Entity::setupCollision(float mass, glm::vec3 halfExtents)
{
	assert(wldPHY);
	if(body)
		wldPHY->remBody(body);
	body = wldPHY->addBody(mass, halfExtents);
	body->setOwner(static_cast<void*>(this));
}

rapidjson::Value Entity::Serialize ( rapidjson::Document& d )
{
    using namespace rapidjson;
    Value entity_value;
    entity_value.SetObject();
    for (auto kv : properties) {
        Property* p = kv.second;
        std::string s = kv.first;
        Value name;
        name.SetString(s.c_str(), s.length());
        entity_value.AddMember(name,  p->Serialize(d), d.GetAllocator());
    }
    return entity_value;
}

void Entity::Deserialize(rapidjson::Value& d)
{
	for(auto it = d.MemberBegin(); it != d.MemberEnd(); ++it)
	{
		string name = it->name.GetString();
		if(name == "class")
        {
            continue;
        }
        if(name == "id")
        {
            int _id = it->value.GetInt();
            enByOldId[_id] = this;
            continue;
        }
		properties[name]->Deserialize(it->value);
	}


    for(int i=0; i<3; i++)
    {
        while(rotationEuler[i] > 180.0f) rotationEuler[i] -= 360.0f;
        while(rotationEuler[i] <-180.0f) rotationEuler[i] += 360.0f;
    }
    rotationQuat = glm::quat(glm::vec3(glm::radians(rotationEuler.x),
                                       glm::radians(rotationEuler.y),
                                       glm::radians(rotationEuler.z)));

    rotationEulerO = rotationEuler;
    rotationQuatO = rotationQuat;

    setPosition(position);
    setRotation(rotationQuat);

    if(editor)
    {
        switchToEditorModel();
    }

    adjustMoving();
}

void Entity::setPosition(glm::vec3 pos)
{
    position = pos;
    if(model)
        model->setPosition(pos);
    if(body)
        body->setPosition(pos);
}

void Entity::setRotation(glm::quat rot)
{
    rotationQuat = rot;
    if(body)
        body->setRotation(rot);
    if(model)
        model->setRotation(rot);
}
