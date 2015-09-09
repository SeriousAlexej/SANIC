#ifndef _WORLD_PHYSICS_H_
#define _WORLD_PHYSICS_H_
#include <glm/vec3.hpp>
#include <vector>
#include "GLDebugDrawer.h"
class SolidBody;
class ModelInstance;
class Camera;
class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

class WorldPhysics
{
public:
	WorldPhysics();
	~WorldPhysics();

	void		update();

	SolidBody*	addBody(float mass, float radius);
	SolidBody*	addBody(float mass, glm::vec3 boxHalfExtents);
	SolidBody*	addBody(float mass, ModelInstance* mi);
	void		remBody(SolidBody*& body);

	void		render(Camera* cam);

private:
    void        Clear();
    void        createPhysics();
    void        deletePhysics();

	GLDebugDrawer								debugDrawer;

	btBroadphaseInterface*						broadphase;
	btDefaultCollisionConfiguration*			collisionConfiguration;
	btCollisionDispatcher*						dispatcher;
	btSequentialImpulseConstraintSolver*		solver;
	btDiscreteDynamicsWorld*					dynamicsWorld;

	std::vector<SolidBody*>			bodies;

	friend class World;
};

#endif
