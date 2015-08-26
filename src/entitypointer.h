#ifndef ENTITYPOINTER_H
#define ENTITYPOINTER_H

#include <string>
#include "properties.h"
#include "entity.h"

class Entity;

class EntityPointer : public Serial, public FromLua
{
public:
    EntityPointer();
    EntityPointer(std::string _name);
    EntityPointer(const EntityPointer& other);
    ~EntityPointer();
    std::string Name() const;
    Entity* Value() const;
    inline int GetCurrentID() const { return enID; }
    bool operator==(const EntityPointer& other) const;
    EntityPointer& operator=(const EntityPointer& other);
    EntityPointer& operator=(Entity* other);


    virtual void Deserialize(rapidjson::Value& d);
    virtual rapidjson::Value Serialize(rapidjson::Document& d);

    virtual void registerLua(LuaUserdata<EntityPointer>& l) {};
private:
    Entity*       penTarget;
    int           enID;
    std::string   name;
};

#endif // ENTITYPOINTER_H
