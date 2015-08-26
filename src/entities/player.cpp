#include <glm/gtx/vector_angle.hpp>
#include "player.h"

ADD_TO_INCUBATOR(Player);

Player::Player()
{
	setClass("Player");
}

void Player::addProperties()
{
    Entity::addProperties();
	setName("Sanic");
}

void Player::initialize()
{
	setupModel("./shaders/smooth.shader",
			   "./models/sanic.obj",
			   "./models/sanic.tga",
			   "", "");
	setupCollision(20.0f, 1.0f);
	switchToModel();
	body->enableTouching();
	camera = wldGFX->getCamera();
	camera->setOffset(glm::vec3(0,2,6));
	camera->setPosition(body->getPosition());
	model->setOffset(glm::vec3(0,-1,0));
	pushState(main);

	addProperties();
}

void Player::adjustMoving()
{
	if(body)
	{
		camera->setPosition(body->getPosition());
		glm::vec3 speed = body->getVelocity();
		if(model && glm::length(speed) > 0.1f)
		{
			glm::vec3 right;
			if(speed.x == 0.0f && speed.z == 0.0f)
			{
				right = glm::vec3(1,0,0);
			} else {
				glm::vec3 speedHorizontal = glm::normalize(glm::vec3(speed.x, 0, speed.z));
				right = glm::normalize(glm::cross(speedHorizontal, glm::vec3(0,1,0)));
			}
			glm::vec3 up = glm::normalize(glm::cross(right, speed));
			glm::vec3 front = glm::normalize(speed);
			glm::mat4 rot(1);

			rot[0] = glm::vec4(-right.x, -right.y, -right.z, 0);
			rot[1] = glm::vec4(up.x, up.y, up.z, 0);
			rot[2] = glm::vec4(-front.x, -front.y, -front.z, 0);
			model->setRotation(rot);
		}
	}
}

Player::~Player()
{
}

STATE Player::main(EntityEvent *ee, Entity *caller)
{
	return;
}
