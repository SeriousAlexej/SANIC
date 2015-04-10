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
    virtual void    Serialize(ostream& o);
    virtual void    Deserialize(istream& i);

private:
	//states
	static STATE main(EntityEvent* ee, Entity* caller);

	//~states
};

#endif // BOX_H_INCLUDED
