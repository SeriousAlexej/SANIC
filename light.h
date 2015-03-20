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
	void		setAmbientColor(glm::vec3 col);
	float		getHotSpot() const { return hotspot; }
	float		getFallOff() const { return falloff; }
	glm::vec3	getDiffuseColor() const { return colorDiffuse; }
	glm::vec3	getAmbientColor() const { return colorAmbient; }

//private:
	glm::vec3	colorDiffuse;
	glm::vec3	colorAmbient;
	float		falloff;
	float		hotspot;
};

#endif
