#ifndef _PLAYER_H_
#define _PLAYER_H_
#include "../entity.h"

class Camera;

class Player : public Entity
{
public:
    FROM_INCUBATOR(Player);

	Player();
	virtual ~Player();
	virtual void	initialize();
	virtual void	adjustMoving();

private:
	//states
	DECLARE_STATE(main);

	//~states
	virtual void            addProperties();

	Camera*		camera;
};

#endif
