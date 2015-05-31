#ifndef ENTITYPOINTER_H
#define ENTITYPOINTER_H

#include <string>
#include "properties.h"
#include "entity.h"

class Entity;

class EntityPointer : public Serial
{
public:
    EntityPointer();
    EntityPointer(std::string _name);
    EntityPointer(const EntityPointer& other);
    ~EntityPointer();
    std::string Name() const;
    Entity* Value() const;
    bool operator==(const EntityPointer& other) const;
    EntityPointer& operator=(const EntityPointer& other);
    EntityPointer& operator=(Entity* other);


    virtual void Deserialize(istream& ostr) {}
    virtual void Serialize(ostream& istr) {}
private:
    Entity*       penTarget;
    int           enID;
    std::string   name;
};

#endif // ENTITYPOINTER_H
