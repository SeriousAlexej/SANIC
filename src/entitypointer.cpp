#include "entity.h"
#include "entitypointer.h"
#include <rapidjson/document.h>

EntityPointer::EntityPointer() :
    private_lud(egg::getInstance().g_lua.CreateUserdata<EntityPointer>(this)),
    null_lud(egg::getInstance().g_lua.CreateUserdata<Entity>(nullptr))
{
    penTarget = nullptr;
    enID = -1;
}

EntityPointer::EntityPointer(Entity* en) :
    private_lud(egg::getInstance().g_lua.CreateUserdata<EntityPointer>(this)),
    null_lud(egg::getInstance().g_lua.CreateUserdata<Entity>(nullptr))
{
    penTarget = en;
    enID = -1;
    if (penTarget) {
        enID = penTarget->getMultipass();
        penTarget->pointerAdded(this);
    }
}

EntityPointer::EntityPointer(const EntityPointer& other) :
    private_lud(egg::getInstance().g_lua.CreateUserdata<EntityPointer>(this)),
    null_lud(egg::getInstance().g_lua.CreateUserdata<Entity>(nullptr))
{
    penTarget = other.penTarget;
    enID = other.enID;
    if(penTarget != nullptr)
    {
        penTarget->pointerAdded(this);
    }
    registerLua();
}

EntityPointer::~EntityPointer()
{
    if(penTarget != nullptr)
    {
        penTarget->pointerLeft(this);
    }

    // TODO: LUA
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

void EntityPointer::registerLua()
{
    Lua& lua = egg::getInstance().g_lua;
    private_lud.Set("GetEntity", lua.CreateFunction<LuaUserdata<Entity>()>([&]() {
        if(penTarget != nullptr) return penTarget->private_lud;
        else return lua.CreateUserdata<Entity>(nullptr);
    }));
    private_lud.Set("SetEntity", lua.CreateFunction<void(LuaUserdata<Entity>)>([&](LuaUserdata<Entity> arg) {
        penTarget = arg.GetPointer();
    }));
}
