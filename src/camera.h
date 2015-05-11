#ifndef _CAMERA_H_
#define _CAMERA_H_
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class Camera
{
public:
	Camera();
	~Camera();

	void	setPerspective(float fov, float ratio, float near, float far);

	const glm::mat4&	getProjectionMatrix();
	const glm::mat4&	getViewMatrix();

	bool		sphereIsVisible(glm::vec4 sphere);//(glm::vec3 pos, float radius);
	bool		pointIsVisible(glm::vec3 point);
	bool		boxIsVisible(glm::vec3 center, glm::vec3 halfSizes);

	void		rotate(float h, float v);
	void		setRotation(float h, float v);
	glm::vec2	getRotation();

	void		moveFront(float dist);
	void		moveRight(float dist);
	void		moveUp(float dist);
	void		translate(glm::vec3 addpos);
	void		setPosition(glm::vec3 pos);
	glm::vec3	getPosition();
	glm::vec3	getFront() { return front; }
	glm::vec3	getRight() { return right; }

	void		setOffset(glm::vec3 _offset);
	glm::vec3	getOffset();

private:
	void		extractFrustum();
	void		updateViewAngles();
	void		updateViewMatrix();

	float		frustum[6][4];
	float		horizontalAngle;
	float		verticalAngle;
	glm::vec3	position;
	glm::vec3	front;
	glm::vec3	right;
	glm::vec3	up;
	glm::mat4	prjMx;
	glm::mat4	viewMx;
	glm::mat4	offset;
};

#endif