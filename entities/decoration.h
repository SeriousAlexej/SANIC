#ifndef _DECORATION_H_
#define _DECORATION_H_
#include "../entity.h"

class Decoration : public Entity
{
public:
	Decoration();
	virtual ~Decoration();
	virtual void initialize();
private:
	static STATE main(EntityEvent* ee, Entity* caller);
};

#endif