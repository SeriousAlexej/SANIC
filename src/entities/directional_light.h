#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H
#include "../entity.h"
#include "../light.h"

class DirectionalLight : public Entity
{
public:
    FROM_INCUBATOR(DirectionalLight);

    DirectionalLight();
    virtual ~DirectionalLight();
    virtual void	initialize();
    virtual void	adjustMoving();

private:
    //states
    DECLARE_STATE(main);
    //~states
	virtual void            editorSelect();
	virtual void            editorUpdate();
	virtual void            renderSelectionIndicator();
	virtual void            addProperties();

    Light*                  lightSource;
};

#endif // DIRECTIONAL_LIGHT_H
