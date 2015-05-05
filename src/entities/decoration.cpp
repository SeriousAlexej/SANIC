#include "decoration.h"

Decoration::Decoration()
{
	setClass("Decoration");
	this->pushState(main);
}

Decoration::~Decoration()
{
}

STATE Decoration::main(EntityEvent* ee, Entity* caller)
{
	return;
}

void Decoration::initialize()
{
}
