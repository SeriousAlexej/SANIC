#include "world_physics.h"

WorldPhysics::WorldPhysics()
{
    broadphase				= new btDbvtBroadphase();
    collisionConfiguration	= new btDefaultCollisionConfiguration();
    dispatcher				= new btCollisionDispatcher(collisionConfiguration);
    solver					= new btSequentialImpulseConstraintSolver;
    dynamicsWorld			= new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);

	if(!broadphase || !collisionConfiguration || !dispatcher || !solver || !dynamicsWorld)
	{
		printf("Failed to init physics\n");
		exit(1);
	}
	//screw accuracy, -9.8 == -10!!!1
    dynamicsWorld->setGravity(btVector3(0,-10,0));

	debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	dynamicsWorld->setDebugDrawer(&debugDrawer);
}

WorldPhysics::~WorldPhysics()
{
	if(dynamicsWorld)
	{
		for (int i=bodies.size()-1; i>=0; i--)
		{
			bodies[i]->removeFromWorld(dynamicsWorld);
		}
		bodies.clear();
		delete dynamicsWorld;
	}
	if(solver)					delete solver;
	if(collisionConfiguration)	delete collisionConfiguration;
	if(dispatcher)				delete dispatcher;
	if(broadphase)				delete broadphase;
}

void WorldPhysics::update()
{
	dynamicsWorld->stepSimulation(g_Delta,10);
}

SolidBody* WorldPhysics::addBody(float mass, float radius)
{
	SolidBody* newBody = new SolidBody(mass, radius);
	if(!newBody || !dynamicsWorld) return NULL;

	newBody->addToWorld(dynamicsWorld);
	bodies.push_back(newBody);
	return newBody;
}

SolidBody* WorldPhysics::addBody(float mass, glm::vec3 boxHalfExtents)
{
	SolidBody* newBody = new SolidBody(mass, boxHalfExtents);
	if(!newBody || !dynamicsWorld) return NULL;

	newBody->addToWorld(dynamicsWorld);
	bodies.push_back(newBody);
	return newBody;
}

SolidBody* WorldPhysics::addBody(float mass, ModelInstance* mi)
{
	SolidBody* newBody = new SolidBody(mass, mi);
	if(!newBody || !dynamicsWorld) return NULL;

	newBody->addToWorld(dynamicsWorld);
	bodies.push_back(newBody);
	return newBody;
}

void WorldPhysics::remBody(SolidBody*& body)
{
	if(!body) return;

	for(int i = bodies.size()-1; i>=0; i--)
	{
		if(bodies[i] == body)
		{
			bodies.erase(bodies.begin() + i);
			body->removeFromWorld(dynamicsWorld);
			delete body;
			body = NULL;
			return;
		}
	}
}

void WorldPhysics::render(Camera* cam)
{
	glUseProgram(0);
	glm::mat4 proj = cam->getProjectionMatrix();
	glm::mat4 view = cam->getViewMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(&proj[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(&view[0][0]);
	dynamicsWorld->debugDrawWorld();
}
