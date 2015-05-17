#ifndef BOX_H_INCLUDED
#define BOX_H_INCLUDED
#include "../entity.h"

class Box : public Entity
{
public:
	Box();
	virtual ~Box();
	virtual void	initialize();
	virtual void	adjustMoving();

private:
	//states
	static STATE main(EntityEvent* ee, Entity* caller);

	//~states
	virtual void            addProperties();
};

#endif // BOX_H_INCLUDED
