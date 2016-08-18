#ifndef ENTITYPOINTER_H
#define ENTITYPOINTER_H

#include <string>
#include "properties.h"
#include "entity.h"

class null_entitypointer : public std::exception
{
    virtual const char* what() const throw()
    {
        return "Trying to dereference NULL EntityPointer";
    }
};

class Entity;

class EntityPointer : public Serial, public FromLua
{
public:
    EntityPointer();
    EntityPointer(std::string _name);
    EntityPointer(const EntityPointer& other);
    ~EntityPointer();
    const std::string& Name();
    inline int GetCurrentID() const { return enID; }
    bool operator==(const EntityPointer& other) const;
    bool operator==(const Entity* other) const;
    EntityPointer& operator=(const EntityPointer& other);
    EntityPointer& operator=(Entity* other);
    operator bool() const { return penTarget != nullptr; }
    Entity* operator*() const { return penTarget; }
    Entity* operator->() const;

    virtual void Deserialize(rapidjson::Value& d);
    virtual rapidjson::Value Serialize(rapidjson::Document& d);

    virtual void registerLua();
    LuaUserdata<EntityPointer> private_lud;
    LuaUserdata<Entity> null_lud;
private:
    Entity*       penTarget;
    int           enID;
    std::string   name;
};

#endif // ENTITYPOINTER_H
