#ifndef _PLAYER_H_
#define _PLAYER_H_
#include "../entity.h"

class Player : public Entity
{
public:
	Player();
	virtual ~Player();
	virtual void	initialize();
	virtual void	adjustMoving();

private:
	//states
	static STATE main(EntityEvent* ee, Entity* caller);

	//~states

	Camera*		camera;
};

#endif