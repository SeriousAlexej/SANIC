#ifndef POINT_LIGHT_H_INCLUDED
#define POINT_LIGHT_H_INCLUDED
#include "entity.h"

class Light;

class PointLight : public Entity
{
public:
    FROM_INCUBATOR(PointLight);

    PointLight();
    virtual ~PointLight();

    virtual void    initialize() override;

protected:
    virtual void    updatePosition() override;

private:
    Light*          lightSource;
};


#endif // POINT_LIGHT_H_INCLUDED
