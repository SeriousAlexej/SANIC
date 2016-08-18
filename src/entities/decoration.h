#ifndef _DECORATION_H_
#define _DECORATION_H_
#include "entity.h"

class Decoration : public Entity
{
public:
    FROM_INCUBATOR(Decoration)

	Decoration();
	virtual ~Decoration();

	virtual void initialize() override;
	virtual void setupAI() override;

private:
	glm::vec3               mdlScale;
	std::string             startAnim;
};

#endif
