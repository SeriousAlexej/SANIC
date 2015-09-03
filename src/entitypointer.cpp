#include "entitypointer.h"
#include <rapidjson/document.h>

extern std::map<EntityPointer*, Entity*> pensOwner;
extern std::vector<EntityPointer*> pensToRetarget;
extern Entity* beingDeserialized;

EntityPointer::EntityPointer()
{
    penTarget = nullptr;
    enID = -1;
    name = "nonamepointer";
}

EntityPointer::EntityPointer(std::string _name)
{
    penTarget = nullptr;
    enID = -1;
    name = _name;
}

EntityPointer::EntityPointer(const EntityPointer& other)
{
    name = other.name;
    penTarget = other.penTarget;
    enID = other.enID;
    if(penTarget != nullptr)
    {
        penTarget->pointerAdded(this);
    }
}

EntityPointer::~EntityPointer()
{
    if(penTarget != nullptr)
    {
        penTarget->pointerLeft(this);
    }
}

std::string EntityPointer::Name() const
{
    return name;
}

bool EntityPointer::operator==(const EntityPointer& other) const
{
    return penTarget == other.penTarget;
}

bool EntityPointer::operator==(const Entity* other) const
{
    return penTarget == other;
}

EntityPointer& EntityPointer::operator=(const EntityPointer& other)
{
    if(penTarget != nullptr)
    {
        penTarget->pointerLeft(this);
    }
    penTarget = other.penTarget;
    enID = other.enID;
    if(penTarget != nullptr)
    {
        penTarget->pointerAdded(this);
    }
    return *this;
}

EntityPointer& EntityPointer::operator=(Entity* other)
{
    if(penTarget != nullptr)
    {
        penTarget->pointerLeft(this);
    }
    penTarget = other;
    if(other != nullptr)
    {
        enID = other->getMultipass();
        other->pointerAdded(this);
    }
    else
    {
        enID = -1;
    }
    return *this;
}

Entity* EntityPointer::operator->() const
{
    if(penTarget == nullptr)
    {
        throw null_entitypointer();
    }
    return penTarget;
}

rapidjson::Value EntityPointer::Serialize(rapidjson::Document& d)
{
	using JsonValue = rapidjson::Value;
	JsonValue val;
	val.SetObject();

	JsonValue name, enid;
	name.SetString(Name().c_str(), Name().length());
	enid.SetInt(enID);

	val.AddMember("name", name, d.GetAllocator());
	val.AddMember("enID", enid, d.GetAllocator());

	return val;
}

void EntityPointer::Deserialize(rapidjson::Value& d)
{
	using JsonValue = rapidjson::Value;
	JsonValue& val = d;
	name = val["name"].GetString();
	enID = val["enID"].GetInt();
	if(enID != -1)
    {
        pensToRetarget.push_back(this);
        pensOwner[this] = beingDeserialized;
    }
}

void EntityPointer::registerLua(LuaUserdata<EntityPointer>& l)
{
    Lua& lua = egg::getInstance().g_lua;
}
