#include <rapidjson/writer.h>
#include "entities/events.h"
#include "entity.h"
#include "solidbody.h"
#include "modelset.h"
#include "quaternion_utils.h"
#include "editorGUI.h"
#include "global.h"
#include "world.h"
#include "world_graphics.h"
#include "world_physics.h"

Entity::Entity() : private_lud(egg::getInstance().g_lua.CreateUserdata<Entity>(this))
{
	_setClass("Entity");
	selected = false;
	wld = nullptr;
	wldGFX = nullptr;
	wldPHY = nullptr;
	hasCollision = false;
	background = false;
	editorOnly = true;
	body = nullptr;
	modelset = nullptr;
	touchable = false;
	rotationEuler = glm::vec3(0,0,0);
	position = glm::vec3(0,0,0);
	relativeTransform = glm::mat4(1);
	oldMatrix = glm::mat4(1);
	mass = 0.0f;
    modelPath = "./models/default.mconf";

    registerProperties(
        "Name",         &name,          nullptr,
        "Rotation",     &rotationEuler, [this]() { setRotation(rotationEuler); },
        "Position",     &position,      [this]() { setPosition(position); },
        "Parent",       &parent,        [this]() { setupParent(); }
    );
}

void Entity::initialize()
{
    //setProperty("Model", FileName("./models/default.mconf"));
    setupModel();
    switchToEditorModel();
}

void Entity::setupAI()
{
    stateStack.push_back(&states["Main"]);
    eventsNextTick.push_back(std::move(std::make_unique<EBegin>()));
}

Entity::~Entity()
{
	if(body)
	{
		assert(wldPHY);
		wldPHY->remBody(body);
	}
	if(modelset)
	{
		assert(wldGFX);
		wldGFX->deleteModelSet(modelset);
	}

	for(int i=children.size()-1; i>=0; i--)
	{
        children[i]->setProperty("Parent", EntityPointer(nullptr));
	}

    if (parent)
        for (int i = parent->children.size() - 1; i >= 0; i--) {
            if (parent->children[i] == this) {
                parent->children.erase(parent->children.begin() + i);
                break;
            }
        }

    for (auto &p : pointAtMe) {
        (*p) = nullptr;
    }
}

void Entity::setSelected(bool s)
{
    selected = s;
    if(modelset)
        modelset->setUseEditorShader(selected);
}

void Entity::setupModel()
{
    assert(wldGFX);
    glm::vec3 lastPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::quat lastRot = glm::quat();
    glm::vec3 lastSca = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 lastOff = glm::vec3(0.0f, 0.0f, 0.0f);

    if(modelset) {
        lastPos = modelset->getPosition();
        lastRot = modelset->getRotationQuat();
        lastSca = modelset->getScale();
        lastOff = modelset->getOffset();
        background = modelset->isBackground();

        wldGFX->deleteModelSet(modelset);
    }

    modelset = wldGFX->createModelSet(modelPath.path);

    modelset->setUseEditorShader(selected);
    modelset->setScale(lastSca);
    modelset->setOffset(lastOff);
    modelset->setBackground(background);
    if(hasCollision || egg::getInstance().g_Editor)
    {
        setupCollision(mass);
    }
    setRotation(lastRot);
    setPosition(lastPos);

    if(editorOnly)
    {
        switchToEditorModel();
    }
}

void Entity::pointerLeft(EntityPointer *pen)
{
    for (int i = pointAtMe.size() - 1; i >= 0; i--) {
        if (pointAtMe[i] == pen) {
            pointAtMe.erase(pointAtMe.begin() + i);
            break;
        }
    }
}

void Entity::pointerAdded(EntityPointer *pen)
{
    assert(pen != nullptr);
    pointAtMe.push_back(pen);
}

void Entity::registerLua(LuaUserdata<Entity> &lua) // TODO: Make it more "generic"
{
	auto statefunction = egg::getInstance().g_lua.CreateFunction<void(luaCallbackFunction)>([&](luaCallbackFunction lcb) {
		//this->pushState(LuaCallback(lcb));
	});
	lua.Set("Add state", statefunction);
}

void Entity::switchToEditorModel()
{
    editorOnly = true;
    if(!egg::getInstance().g_Editor)
	{
		if(modelset)
			modelset->deactivate();
		if(body)
			body->deactivate();
	}
}

void Entity::switchToModel()
{
	editorOnly = false;
	if(modelset)
		modelset->activate();
	if(body)
		body->activate();
}

void Entity::destroy()
{
	assert(wld);
	wld->removeEntity(this);
}

void Entity::update()
{
	if(!editorOnly)
	{
		if(modelset && body)
		{ //sync physical and graphical worlds :3
			modelset->setPosition(body->getPosition());
		}
	}
	updatePosition();
}

void Entity::updateAI()
{
    if (!stateStack.empty()) {

        while (!events.empty()) {
            int numPops = 0;
            int sz = stateStack.size();

            for (int i = sz-1; i >= 0; --i) {
                if (!stateStack[i]->CanHandle(events.back()->code)) {
                    numPops++;
                } else {
                    break;
                }
            }

            if (numPops == sz)  {
                events.pop_back();
                continue;
            }

            for (; numPops > 0; --numPops) {
                stateStack.pop_back();
            }
            break;
        }

        stateStack.back()->Process(this, egg::getInstance().g_Delta, events);
    } else {
        events.clear();
        destroy();
    }

    switch (stateChange) {
        case SC_SWAP:
            stateStack.pop_back();

        case SC_PUSH:
            timerSeconds  =  0.0f;
            if (nextState) {
                stateStack.push_back(nextState);
            }
            break;

        case SC_POP:
            timerSeconds  =  0.0f;
            if (!stateStack.empty()) {
                stateStack.pop_back();
            }
            break;

        case SC_NOCHANGE:
            if (timerSeconds > 0.0f) {
                timerSeconds -= egg::getInstance().g_Delta;

                if (timerSeconds <= 0.0f) {
                    eventsNextTick.push_back(std::move(std::make_unique<ETimer>()));
                }
            }
            break;

        default:
            assert(false);
    }
    stateChange = SC_NOCHANGE;
    nextState = nullptr;

    while(!lonely())
    {
        auto touchEvent = MAKEEVENT(ETouch);
        touchEvent->who = static_cast<Entity*>(popToucher());
        eventsNextTick.push_back(std::move(touchEvent));
    }

    for (int j=0, sz=eventsNextTick.size(); j<sz; ++j) {
        for (int i=stateStack.size()-1; i>=0; --i) {
            if (stateStack[i]->CanHandle(eventsNextTick[j]->code)) {
                if (_phase != 0) {
                    _phase = 0;
                    _waitTime = 0.0f;
                    if (!events.empty()) {
                        events.pop_back();
                    }
                }
                events.push_back(std::move(eventsNextTick[j]));
                break;
            }
        }
    }
    eventsNextTick.clear();
}

void Entity::setTimer(float wTime)
{
    if (wTime > 0.0f && stateStack.back()->CanHandle(ETimer::_code)) {
        timerSeconds = wTime;
    }
}

void Entity::sendEvent(EventPtr e)
{
    eventsNextTick.push_back(std::move(e));
}

bool Entity::pushState(EntityState* es)
{
    stateChange = SC_PUSH;
    nextState = es;
    return true;
}

bool Entity::popState()
{
    stateChange = SC_POP;
    return true;
}

bool Entity::swapState(EntityState* es)
{
    stateChange = SC_SWAP;
    nextState = es;
    return true;
}

void Entity::parentMoved()
{
    glm::mat4 newPos = parent->modelset->getMatrix() * relativeTransform;
    position = glm::vec3(newPos[3][0],newPos[3][1],newPos[3][2]);
    setRotation(glm::quat(newPos));
    setPosition(position);
    oldMatrix = modelset->getMatrix();
    for(int i=children.size()-1; i>=0; i--)
    {
        children[i]->parentMoved();
    }
}

void Entity::updatePosition()
{
	if(modelset)
	{
        const glm::mat4& newMatrix = modelset->getMatrix();
        if(newMatrix != oldMatrix)
        {
            if(parent)
            {
                relativeTransform = glm::inverse(parent->modelset->getMatrix()) * modelset->getMatrix();
            }
            for (int i = children.size() - 1; i >= 0; i--) {
                children[i]->parentMoved();
            }
            oldMatrix = newMatrix;
        }
    }
}

bool Entity::childrenContain(const Entity *e) const
{
    for (int i = children.size() - 1; i >= 0; i--) {
        if (children[i] == e || children[i]->childrenContain(e))
            return true;
    }
    return false;
}

void Entity::setupParent()
{
    if(parentOld == parent) {
        return;
    }
    if(modelset == nullptr) {
        parent = parentOld = nullptr;
        return;
    }
    if(parent && parent->modelset == nullptr) {
        parent = parentOld;
        return;
    }
    if(parent == this || childrenContain(parent.Get())) {
        parent = parentOld;
        return;
    }
    if (parentOld)
        for (int i = parentOld->children.size() - 1; i >= 0; i--) {
            if (parentOld->children[i] == this) {
                parentOld->children.erase(parentOld->children.begin() + i);
                break;
            }
        }
    parentOld = parent;
    if (parent) {
        parent->children.push_back(this);
        relativeTransform = glm::inverse(parent->modelset->getMatrix()) * modelset->getMatrix();
    }
}

const EntityPointer& Entity::getParent() const
{
    return parent;
}

void Entity::setupCollision(float mass)
{
    assert(modelset);
    if(modelset->collision == ModelSet::CT_BOX)
    {
        setupCollision(mass, modelset->collisionHalfBox);
    } else
    if(modelset->collision == ModelSet::CT_SPHERE)
    {
        setupCollision(mass, modelset->collisionRadius);
    } else
    if(modelset->collision == ModelSet::CT_MESH)
    {
        assert(wldPHY);
        if(body)
            wldPHY->remBody(body);
        body = wldPHY->addBody(mass, modelset->collisionMesh); //NULL is handled iside the method
        body->setOwner(static_cast<void*>(this));
    }
    body->setOffset(modelset->collisionOffset);
}

void Entity::setupCollision(float mass, float &radius)
{
    assert(wldPHY);
    if (body)
        wldPHY->remBody(body);
    body = wldPHY->addBody(mass, radius);
    body->setOwner(static_cast<void *>(this));
}

void Entity::setupCollision(float mass, glm::vec3 &halfExtents)
{
    assert(wldPHY);
    if (body)
        wldPHY->remBody(body);
    body = wldPHY->addBody(mass, halfExtents);
    body->setOwner(static_cast<void *>(this));
}

rapidjson::Value Entity::Serialize(rapidjson::Document &d)
{
    using namespace rapidjson;
    Value entity_value;
    entity_value.SetObject();
    for (auto& kv : properties) {
        Property& p = kv.second;
        std::string s = kv.first;
        Value name;
        name.SetString(s.c_str(), s.length(), d.GetAllocator());
        entity_value.AddMember(name,  p.Serialize(d), d.GetAllocator());
    }

    Value aiStates, aiName, aiSize, aiPhase, aiWaitTime, aiTimer;
    aiStates.SetArray();
    aiName.SetString("AI", 2, d.GetAllocator());
    std::vector<std::string> stateStackString(stateStack.size(), "");

    aiSize.SetInt(stateStack.size() + 4);
    aiPhase.SetInt((int)_phase);
    aiWaitTime.SetFloat(_waitTime);
    aiTimer.SetFloat(timerSeconds);

    aiStates.PushBack(aiSize, d.GetAllocator());
    aiStates.PushBack(aiPhase, d.GetAllocator());
    aiStates.PushBack(aiWaitTime, d.GetAllocator());
    aiStates.PushBack(aiTimer, d.GetAllocator());

    for(auto it=states.begin(); it!=states.end(); it++) {
        const std::string& stateName = it->first;
        const EntityState* stateAddr = &(it->second);

        for(int i=stateStack.size()-1; i>=0; --i) {
            if(stateStack[i] == stateAddr) {
                stateStackString[i] = stateName;
            }
        }
    }

    for(int i=0,sz=stateStackString.size(); i<sz; ++i) {
        Value stName;
        stName.SetString(stateStackString[i].c_str(), stateStackString[i].length(), d.GetAllocator());
        aiStates.PushBack(stName, d.GetAllocator());
    }

    entity_value.AddMember(aiName, aiStates, d.GetAllocator());

    return entity_value;
}

rapidjson::Value Entity::SerializeForCopying(rapidjson::Document& d)
{
    using namespace rapidjson;
    Value entity_value;
    entity_value.SetObject();
    for (auto& kv : properties) {
        Property& p = kv.second;
        std::string s = kv.first;
        //do not copy pointers and position
        if(s == "Position" || p.m_tid == typeid(EntityPointer).hash_code())
        {
            continue;
        }
        Value name;
        name.SetString(s.c_str(), s.length(), d.GetAllocator());
        entity_value.AddMember(name,  p.Serialize(d), d.GetAllocator());
    }
    return entity_value;
}

void Entity::Deserialize(rapidjson::Value& d)
{
    using namespace rapidjson;
    for (auto it = d.MemberBegin(); it != d.MemberEnd(); ++it) {
        std::string name = it->name.GetString();

        if (name == "class" || name == "id" ||
            name == "Position" || name == "Rotation") { //those are set in World
            continue;
        }

        if (name == "AI") {
            stateStack.clear();
            Value &aiArray = it->value;

            _phase = (unsigned)aiArray[1].GetInt();
            _waitTime = aiArray[2].GetFloat();
            timerSeconds = aiArray[3].GetFloat();

            for(int i=4,sz=aiArray[0].GetInt(); i<sz; ++i) {
                stateStack.push_back(&states[ std::string(aiArray[i].GetString()) ]);
            }
            continue;
        }
        properties[name].Deserialize(it->value);
    }
}

void Entity::setPosition(glm::vec3 pos)
{
    position = pos;
    if(modelset)
        modelset->setPosition(pos);
    if(body)
        body->setPosition(pos);
}

void Entity::setRotation(glm::vec3 rot)
{
    rotationEuler = rot;
    rotationEuler[0] -= 360.0f*(int(rotationEuler[0])/360);
    rotationEuler[1] -= 360.0f*(int(rotationEuler[1])/360);
    rotationEuler[2] -= 360.0f*(int(rotationEuler[2])/360);
    glm::quat rotationQuat = glm::quat(glm::vec3(glm::radians(rotationEuler.x),
                                                 glm::radians(rotationEuler.y),
                                                 glm::radians(rotationEuler.z)));
    if (body)
        body->setRotation(rotationQuat);
    if (modelset)
        modelset->setRotation(rotationQuat);
}

void Entity::setRotation(glm::quat rot)
{
    rotationEuler = glm::eulerAngles(rot) * glm::one_over_pi<float>() * 180.0f;
    if (body)
        body->setRotation(rot);
    if (modelset)
        modelset->setRotation(rot);
}
