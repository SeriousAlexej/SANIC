#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H
#include "entity.h"

class Light;

class DirectionalLight : public Entity
{
public:
    FROM_INCUBATOR(DirectionalLight);

    DirectionalLight();
    virtual ~DirectionalLight();

    virtual void	initialize() override;

protected:
    virtual void    updatePosition() override;

private:
    Light*          lightSource;
};

#endif // DIRECTIONAL_LIGHT_H
