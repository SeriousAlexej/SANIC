#ifndef _LIGHT_H_
#define _LIGHT_H_
#include "basic.h"

class Light : public Movable
{
public:
	Light();
	virtual ~Light();

	void		setHotSpot(float val);
	void		setFallOff(float val);
	void		setDiffuseColor(glm::vec3 col);
	float		getHotSpot() const { return hotspot; }
	float		getFallOff() const { return falloff; }
	float       getIntensity() const { return intensity; }
	glm::vec3	getDiffuseColor() const { return colorDiffuse; }

//private:
	glm::vec3	colorDiffuse;
	float		falloff;
	float		hotspot;
	float       intensity;
};

#endif
