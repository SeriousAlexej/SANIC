#include "basic.h"

std::vector<int> Unique::registeredMultipasses;

Movable::Movable()
{
	posMx = glm::translate(glm::vec3(0,0,0));
	rotMx = glm::rotate(0.0f, glm::vec3(1,0,0));
	scaMx = glm::scale(glm::vec3(1,1,1));
	updateModelMatrix();
}

Movable::~Movable() {}

glm::vec3 Movable::getPosition()
{
	return glm::vec3(posMx[3].x, posMx[3].y, posMx[3].z);
}

glm::mat4 Movable::getRotation()
{
	return rotMx;
}

glm::vec3 Movable::getScale()
{
	return glm::vec3(scaMx[0].x, scaMx[1].y, scaMx[2].z);
}

const glm::mat4& Movable::getMatrix()
{
	return mdlMx;
}

void Movable::setPosition(glm::vec3 pos)
{
	posMx = glm::translate(pos);
	updateModelMatrix();
}

void Movable::setRotation(glm::vec3 rot)
{
	glm::mat4 tmpMx1, tmpMx2, tmpMx3;
	tmpMx1 = glm::rotate(rot.x, glm::vec3(1,0,0));
	tmpMx2 = glm::rotate(rot.y, glm::vec3(0,1,0));
	tmpMx3 = glm::rotate(rot.z, glm::vec3(0,0,1));
	rotMx = tmpMx3 * tmpMx2 * tmpMx1;
	updateModelMatrix();
}

void Movable::setRotation(float angle, glm::vec3 dir)
{
	rotMx = glm::rotate(angle, dir);
	updateModelMatrix();
}

void Movable::setScale(glm::vec3 sca)
{
	scaMx = glm::scale(sca);
	updateModelMatrix();
}

void Movable::translate(glm::vec3 addpos)
{
	posMx[3].x += addpos.x;
	posMx[3].y += addpos.y;
	posMx[3].z += addpos.z;
	updateModelMatrix();
}

void Movable::rotate(glm::vec3 addrot)
{
	glm::mat4 tmpMx1, tmpMx2, tmpMx3;
	tmpMx1 = glm::rotate(addrot.x, glm::vec3(1,0,0));
	tmpMx2 = glm::rotate(addrot.y, glm::vec3(0,1,0));
	tmpMx3 = glm::rotate(addrot.z, glm::vec3(0,0,1));
	rotMx = tmpMx3 * tmpMx2 * tmpMx1 * rotMx;
	updateModelMatrix();
}

void Movable::rotate(float angle, glm::vec3 dir)
{
	glm::mat4 tmpMx;
	tmpMx = glm::rotate(angle, dir);
	rotMx = tmpMx * rotMx;
	updateModelMatrix();
}

void Movable::scale(glm::vec3 addsca)
{
	glm::mat4 addscaMx;
	addscaMx = glm::scale(addsca);
	scaMx = addscaMx * scaMx;
	updateModelMatrix();
}

void Movable::updateModelMatrix()
{
	mdlMx = posMx * rotMx * scaMx;
}
