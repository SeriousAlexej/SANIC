#include "box.h"

Box::Box()
{
	setClass("Box");
	setName("Box");
	setTranslatedByBody(false);
	orientationType = BY_BODY;
}

void Box::initialize()
{
	setupModel("./shaders/fullbright.vsh",
		       "./shaders/fullbright.fsh",
			   "./models/box.obj",
			   "./models/uv_checker.jpg",
			   "");
	setupCollision(1.0f, glm::vec3(0.5f, 0.5f, 0.5f));
	switchToModel();
    body->setOffset(glm::vec3(0,0.5,0));
	pushState(main);
}

void Box::adjustMoving()
{
}

Box::~Box()
{
}

STATE Box::main(EntityEvent *ee, Entity *caller)
{
	return;
}