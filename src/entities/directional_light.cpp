#include "directional_light.h"
#include "light.h"
#include "global.h"
#include "world_graphics.h"

ADD_TO_INCUBATOR(DirectionalLight);

DirectionalLight::DirectionalLight()
{
	lightSource = nullptr;
	modelPath = "./models/editor/dirLight.mconf";
	name = "Directional Light";
}

DirectionalLight::~DirectionalLight()
{
    assert(wldGFX);
    wldGFX->deleteDirLight();
}

void DirectionalLight::initialize()
{
    Entity::initialize();
	lightSource = wldGFX->createDirLight();

    registerProperties("Ambient Color", &(lightSource->colorAmbient), nullptr,
                       "Diffuse Color", &(lightSource->colorDiffuse), nullptr);
}

void DirectionalLight::updatePosition()
{
    if(lightSource && modelset)
    {
        lightSource->setRotation(modelset->getRotation());
    }
}
