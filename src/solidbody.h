#ifndef _SOLID_BODY_H_
#define _SOLID_BODY_H_
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "basic.h"
#include "modelinstance.h"
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"
#include "BulletCollision/CollisionDispatch/btInternalEdgeUtility.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/CollisionShapes/btTriangleShape.h"

enum CollisionType { CollBox, CollSphere, CollMesh };

class SolidBody
{
public:
	SolidBody(float _mass, float radius); // sphere
	SolidBody(float _mass, glm::vec3 boxHalfExtents); // box
	SolidBody(float _mass, ModelInstance* mi); // mesh
	~SolidBody();

	void	activate();
	void	deactivate();

	void		rotate(glm::vec3 rot, bool absolute = true);
	void		translate(glm::vec3 pos);
	void        setOffset(glm::vec3 off);
	glm::vec3   getOffset() const;
	void		setPosition(glm::vec3 pos);
	void		setRotation(glm::vec3 rot);
	void		setRotation(glm::quat q);
	void		setVelocity(glm::vec3 vel);
	void		setAngularVelocity(glm::vec3 vel);
	void		addVelocity(glm::vec3 vel);
	void		addAngularVelocity(glm::vec3 vel);
	glm::vec3	getPosition();
	glm::vec3	getRotation();
	glm::quat	getRotationQuat();
	glm::vec3	getVelocity();
	glm::vec3	getAngularVelocity();
	void		teleport(glm::vec3 pos, glm::vec3 rot);
	void		stopMoving();
	void		stopRotating();

	void		setOwner(void* owner);
	void*		getOwner();

	CollisionType	getType() { return collisionType; }

private:
	void	createBodyFromShape(btScalar _mass);
	void	addToWorld();
	void	removeFromWorld();
	void	setWorld(btDiscreteDynamicsWorld* myworld);

	CollisionType	collisionType;
	float			mass;

    btVector3                   offset;
	btCollisionShape*			collShape;
	btRigidBody*				rigidBody;
	btCompoundShape*            compoundShape;
	btTriangleIndexVertexArray*	indexVertexArrays;
	btBvhTriangleMeshShape*		trimeshShape;
	btTriangleInfoMap*          triangleInfoMap;

	btDiscreteDynamicsWorld*	world;

	friend class WorldPhysics;
};

#endif
