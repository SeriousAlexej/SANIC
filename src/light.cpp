#include "light.h"

Light::Light()
{
    intensity = 1.0f;
	hotspot = 0.0f;
	falloff = 10.0f;
	colorDiffuse = glm::vec3(1,1,1);
	colorAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
}

Light::~Light()
{
}


void Light::setHotSpot(float val)
{
	if(val < 0.0f) val = 0.0f;
	if(val > falloff) val = falloff;
	hotspot = val;
}

void Light::setFallOff(float val)
{
	if(val < 0.0f) val = 0.0f;
	falloff = val;
}

void Light::setDiffuseColor(glm::vec3 col)
{
	for(short i=0; i<3; i++)
	{
		if(col[i] > 1.0f) col[i] = 1.0f;
		else
		if(col[i] < 0.0f) col[i] = 0.0f;
	}
	colorDiffuse = col;
}

void Light::setAmbientColor(glm::vec3 col)
{
	for(short i=0; i<3; i++)
	{
		if(col[i] > 1.0f) col[i] = 1.0f;
		else
		if(col[i] < 0.0f) col[i] = 0.0f;
	}
	colorAmbient = col;
}

