#include "entity.h"
#include "entitypointer.h"
#include <rapidjson/document.h>

EntityPointer::EntityPointer()
{
    penTarget = nullptr;
    enID = -1;
}

EntityPointer::EntityPointer(Entity* en)
{
    penTarget = en;
    enID = -1;
    if (penTarget) {
        enID = penTarget->getMultipass();
        penTarget->pointerAdded(this);
    }
}

EntityPointer::EntityPointer(const EntityPointer& other)
{
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

void EntityPointer::registerLua(LuaUserdata<EntityPointer>& l)
{
    Lua& lua = egg::getInstance().g_lua;
    auto constructor = lua.CreateFunction<LuaUserdata<EntityPointer>()>([&]() {
        auto lud = lua.CreateUserdata<EntityPointer>(new EntityPointer);
        lud.Set("GetEntity", lua.CreateFunction<LuaUserdata<Entity>()>([&]() {
            auto eud = lua.CreateUserdata<Entity>(penTarget);
            if(penTarget != nullptr) penTarget->registerLua(eud);
            return eud;
        }));
        lud.Set("SetEntity", lua.CreateFunction<void(LuaUserdata<Entity>)>([&](LuaUserdata<Entity> arg) {
            penTarget = arg.GetPointer();
        }));
        return lud;
    });
    auto table = lua.CreateTable();
    table.Set("new", constructor);
    lua.GetGlobalEnvironment().Set("EntityPointer", table);
}
