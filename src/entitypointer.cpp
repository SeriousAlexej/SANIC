#include "entitypointer.h"

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

Entity* EntityPointer::Value() const
{
    return penTarget;
}

bool EntityPointer::operator==(const EntityPointer& other) const
{
    return penTarget == other.penTarget;
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
