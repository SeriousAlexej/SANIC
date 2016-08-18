#include "pointlight.h"
#include "light.h"
#include "world_graphics.h"

ADD_TO_INCUBATOR(PointLight);

PointLight::PointLight()
{
	lightSource = nullptr;
	modelPath = "./models/editor/pointLight.mconf";
	name = "Light";
}

PointLight::~PointLight()
{
    assert(wldGFX);
    wldGFX->deleteLight(lightSource);
}

void PointLight::initialize()
{
    Entity::initialize();
	lightSource = wldGFX->createLight();

	registerProperties("Diffuse Color", &(lightSource->colorDiffuse), nullptr,
                       "Fall Off",      &(lightSource->falloff),      nullptr,
                       "Hot Spot",      &(lightSource->hotspot),      nullptr,
                       "Intentsity",    &(lightSource->intensity),    nullptr);
}

void PointLight::updatePosition()
{
    Entity::updatePosition();
    if(lightSource && modelset)
    {
        lightSource->setPosition(modelset->getPosition());
    }
}

