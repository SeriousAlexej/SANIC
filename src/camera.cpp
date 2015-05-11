#include "camera.h"

Camera::Camera()
{
	this->setPerspective(90.0f, 4.0f/3.0f, 0.1f, 100.0f);
	horizontalAngle = 0.0f;
	verticalAngle = -0.3f;
	position = glm::vec3(0,0,0);
	offset = glm::translate(glm::vec3(0,0,0));
	updateViewAngles(); //this calls updateMatrix && extractFrustum
}

Camera::~Camera()
{
}

void Camera::setPerspective(float fov, float ratio, float near, float far)
{
	prjMx = glm::perspective(fov, ratio, near, far);
	extractFrustum();
}

const glm::mat4& Camera::getProjectionMatrix()
{
	return prjMx;
}

const glm::mat4& Camera::getViewMatrix()
{
	return viewMx;
}

bool Camera::sphereIsVisible(glm::vec4 sphere)//(glm::vec3 pos, float radius)
{
	int p;
	for(p=0; p<6; p++)
		if( frustum[p][0] * sphere.x + frustum[p][1] * sphere.y + frustum[p][2] * sphere.z + frustum[p][3] <= -sphere.w)
			return false;
	return true;
}

bool Camera::pointIsVisible(glm::vec3 point)
{
	int p;
	for(p=0; p<6; p++)
		if( frustum[p][0] * point.x + frustum[p][1] * point.y + frustum[p][2] * point.z + frustum[p][3] <= 0 )
			return false;
	return true;
}

bool Camera::boxIsVisible(glm::vec3 center, glm::vec3 halfSizes)
{
	return pointIsVisible(center) || 
		pointIsVisible(center + halfSizes) ||
		pointIsVisible(center - halfSizes) ||
		pointIsVisible(center + glm::vec3(-halfSizes.x, halfSizes.y, halfSizes.z)) ||
		pointIsVisible(center + glm::vec3(halfSizes.x, -halfSizes.y, halfSizes.z)) ||
		pointIsVisible(center + glm::vec3(halfSizes.x, halfSizes.y, -halfSizes.z)) ||
		pointIsVisible(center + glm::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z)) ||
		pointIsVisible(center + glm::vec3(-halfSizes.x, halfSizes.y, -halfSizes.z)) ||
		pointIsVisible(center + glm::vec3(halfSizes.x, -halfSizes.y, -halfSizes.z));
}

void Camera::extractFrustum()
{
	float	t;
	
	glm::mat4 clip = prjMx * viewMx;

	// Extract the RIGHT clipping plane
	frustum[0][0] = clip[0][3] - clip[0][0];
	frustum[0][1] = clip[1][3] - clip[1][0];
	frustum[0][2] = clip[2][3] - clip[2][0];
	frustum[0][3] = clip[3][3] - clip[3][0];

	// Extract the LEFT clipping plane
	frustum[1][0] = clip[0][3] + clip[0][0];
	frustum[1][1] = clip[1][3] + clip[1][0];
	frustum[1][2] = clip[2][3] + clip[2][0];
	frustum[1][3] = clip[3][3] + clip[3][0];

	// Extract the BOTTOM clipping plane
	frustum[2][0] = clip[0][3] + clip[0][1];
	frustum[2][1] = clip[1][3] + clip[1][1];
	frustum[2][2] = clip[2][3] + clip[2][1];
	frustum[2][3] = clip[3][3] + clip[3][1];

	// Extract the TOP clipping plane
	frustum[3][0] = clip[0][3] - clip[0][1];
	frustum[3][1] = clip[1][3] - clip[1][1];
	frustum[3][2] = clip[2][3] - clip[2][1];
	frustum[3][3] = clip[3][3] - clip[3][1];

	// Extract the FAR clipping plane
	frustum[4][0] = clip[0][3] - clip[0][2];
	frustum[4][1] = clip[1][3] - clip[1][2];
	frustum[4][2] = clip[2][3] - clip[2][2];
	frustum[4][3] = clip[3][3] - clip[3][2];

	// Extract the NEAR clipping plane
	frustum[5][0] = clip[0][3] + clip[0][2];
	frustum[5][1] = clip[1][3] + clip[1][2];
	frustum[5][2] = clip[2][3] + clip[2][2];
	frustum[5][3] = clip[3][3] + clip[3][2];

	// Normalize it
	for(int i=0; i<6; ++i)
	{
		t = (float) sqrt( frustum[i][0] * frustum[i][0] + frustum[i][1] * frustum[i][1] + frustum[i][2] * frustum[i][2] );
		frustum[i][0] /= t;
		frustum[i][1] /= t;
		frustum[i][2] /= t;
		frustum[i][3] /= t;
	}
}

void Camera::rotate(float h, float v)
{
	horizontalAngle += h;
	verticalAngle += v;
	updateViewAngles();
}

void Camera::setRotation(float h, float v)
{
	horizontalAngle = h;
	verticalAngle = v;
	updateViewAngles();
}

glm::vec2 Camera::getRotation()
{
	return glm::vec2(horizontalAngle, verticalAngle);
}

void Camera::moveFront(float dist)
{
	position += front * dist;
	updateViewMatrix();
}

void Camera::moveRight(float dist)
{
	position += right * dist;
	updateViewMatrix();
}

void Camera::moveUp(float dist)
{
	position += up * dist;
	updateViewMatrix();
}

void Camera::translate(glm::vec3 addpos)
{
	position += addpos;
	updateViewMatrix();
}

void Camera::setPosition(glm::vec3 pos)
{
	position = pos;
	updateViewMatrix();
}

glm::vec3 Camera::getPosition()
{
	return position;
}

void Camera::setOffset(glm::vec3 _offset)
{
	offset = glm::translate(-_offset);
	updateViewMatrix();
}

glm::vec3 Camera::getOffset()
{
	return glm::vec3(offset[3].x, offset[3].y, offset[3].z);
}

void Camera::updateViewAngles()
{
	if(verticalAngle > 3.14f*0.5f) verticalAngle = 3.14f*0.5f;
	else
	if(verticalAngle < -3.14f*0.5f) verticalAngle = -3.14f*0.5f;

	while(horizontalAngle > 3.14f*2.0f) horizontalAngle -= 3.14f*2.0f;
	while(horizontalAngle < 0.0f) horizontalAngle += 3.14f*2.0f;

	front = glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
		);
	right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f),
		0,
		cos(horizontalAngle - 3.14f/2.0f)
		);
	up = glm::cross( right, front );
	updateViewMatrix();
}

void Camera::updateViewMatrix()
{
	viewMx = offset * glm::lookAt(position, position+front, up);
	extractFrustum();
}
