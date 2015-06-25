#ifndef POINT_LIGHT_H_INCLUDED
#define POINT_LIGHT_H_INCLUDED
#include "../entity.h"
#include "../light.h"

class PointLight : public Entity
{
public:
    FROM_INCUBATOR(PointLight);

    PointLight();
    virtual ~PointLight();
	virtual void	initialize();
	virtual void	adjustMoving();

private:
    //states
    static STATE main(EntityEvent* ee, Entity* caller);
    //~states
	virtual void            editorSelect();
	virtual void            editorUpdate();
	virtual void            renderSelectionIndicator();
	virtual void            addProperties();

    Light*                  lightSource;
};


#endif // POINT_LIGHT_H_INCLUDED
