#ifndef _WORLD_PHYSICS_H_
#define _WORLD_PHYSICS_H_
#include "solidbody.h"
#include "GLDebugDrawer.h"

extern float g_Delta;

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
	//void		clearGarbage();

	GLDebugDrawer								debugDrawer;

	btBroadphaseInterface*						broadphase;
	btDefaultCollisionConfiguration*			collisionConfiguration;
	btCollisionDispatcher*						dispatcher;
	btSequentialImpulseConstraintSolver*		solver;
	btDiscreteDynamicsWorld*					dynamicsWorld;

	std::vector<SolidBody*>			bodies;
	//float							tmLastGarbageClean;
	//float							garbageCleanInterval;
};

#endif