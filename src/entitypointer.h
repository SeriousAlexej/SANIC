#ifndef ENTITYPOINTER_H
#define ENTITYPOINTER_H

#include <luacppinterface.h>
#include "basic.h"
#include <functional>

class null_entitypointer : public std::exception
{
    virtual const char* what() const throw()
    {
        return "Trying to dereference NULL EntityPointer";
    }
};

class Entity;
class FromLua;

class EntityPointer : public FromLua
{
    friend class Entity;
public:
    EntityPointer();
    EntityPointer(Entity* en);
    EntityPointer(const EntityPointer& other);
    virtual ~EntityPointer();
    inline const Entity* Get() const { return penTarget; }
    inline int GetCurrentID() const { return enID; }
    bool operator==(const EntityPointer& other) const;
    bool operator==(const Entity* other) const;
    EntityPointer& operator=(const EntityPointer& other);
    EntityPointer& operator=(Entity* other);
    operator bool() const { return penTarget != nullptr; }
    Entity* operator*() const { return penTarget; }
    Entity* operator->() const;

    virtual void registerLua();

private:
    Entity*       penTarget;
    int           enID;
    LuaUserdata<EntityPointer> private_lud;
};

#endif // ENTITYPOINTER_H
