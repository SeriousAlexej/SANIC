#ifndef _CAMERA_H_
#define _CAMERA_H_
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glew.h>

class Camera
{
public:
	Camera();
	~Camera();

	enum ShadowRenderMode
	{
	    LQ = 0,
	    HQ = 1
	};

	void	setPerspective(float fovxdeg, float ratio, float near, float _far);

	inline const glm::mat4&	getProjectionMatrix() const { return prjMx; }
	inline const glm::mat4&	getViewMatrix() const { return viewMx; }
	inline const glm::mat4& getBcgMatrix() const { return bcgMx; }
	inline const glm::mat4& getShadowPVMatrix() const { return shadowPVMatrix; }
	inline const glm::mat4& getShadowLQPVMatrix() const { return shadowLQPVMatrix; }
	const glm::mat4&        getCurrentShadowPVMatrix() const;
	inline GLuint           getShadowTexture() const { return shadowTexture; }
	inline GLuint           getShadowLQTexture() const { return shadowTextureLQ; }
	inline float            getHQShadowBorder() const { return shadowCubeHSide*0.5f; }

	inline void             setShadowRenderMode(ShadowRenderMode rm) { renMode = rm; }

	bool		sphereIsVisible(glm::vec4 sphere);//(glm::vec3 pos, float radius);
	bool		pointIsVisible(glm::vec3 point);
	bool        sphereIsVisibleForShadow(glm::vec4 sphere);

	void		rotate(float h, float v);
	void		setRotation(float h, float v);
	glm::vec2	getRotation();

	void		moveFront(float dist);
	void		moveRight(float dist);
	void		moveUp(float dist);
	void		translate(glm::vec3 addpos);
	void		setPosition(glm::vec3 pos);
	glm::vec3	getPosition();
	inline glm::vec3	getFront() const { return front; }
	inline glm::vec3	getRight() const { return right; }
	inline glm::vec3    getUp()    const { return up;    }

	void		setOffset(glm::vec3 _offset);
	glm::vec3	getOffset();

	void        preShadowRender();
	void        postShadowRender() const;
	void        setShadowViewMatrix(glm::mat4 &dirLight);

private:
	void		extractFrustum();
	void        extractShadowFrustum();
	void		updateViewAngles();
	void		updateViewMatrix();

	void        createFrameBuffersAndTextures();
	void        getFbTexRes(GLuint*& fb, GLuint*& tex, GLsizei& res);

	float		frustum[6][4];
	float       shadowFrustum[6][4];
	float		horizontalAngle;
	float		verticalAngle;
	float       far;
	glm::vec3	position;
	glm::vec3   lastPosition;
	glm::vec3	front;
	glm::vec3	right;
	glm::vec3	up;
	glm::mat4	prjMx;
	glm::mat4	viewMx;
	glm::mat4   bcgMx;
	glm::mat4	offset;

    float       shadowCubeHSide;
    float       shadowCubeHSideHQ;

	glm::mat4   shadowProjMatrix;
	glm::mat4   shadowLQProjMatrix;

	glm::mat4   shadowPVMatrix;
	glm::mat4   shadowLQPVMatrix;

	GLuint      shadowFramebuffer;
	GLuint      shadowFramebufferLQ;

	GLuint      shadowTexture;
	GLuint      shadowTextureLQ;

	ShadowRenderMode renMode;
};

#endif
