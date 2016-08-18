#include "world_physics.h"
#include "entity.h"
#include "solidbody.h"
#include "camera.h"

extern ContactProcessedCallback		gContactProcessedCallback;

static bool CustomProcessedCallback(btManifoldPoint& cp, void* body0,void* body1)
{
	btRigidBody* rb0 = static_cast<btRigidBody*>(body0);
	btRigidBody* rb1 = static_cast<btRigidBody*>(body1);

	void* up0 = rb0->getUserPointer();
	void* up1 = rb1->getUserPointer();

	if(up0 != NULL && up1 != NULL)
	{
		Entity* t0 = static_cast<Entity*>(up0);
		Entity* t1 = static_cast<Entity*>(up1);
		if(t0->isTouchable())
            t0->touch(up1);
        if(t1->isTouchable())
            t1->touch(up0);
	}
	return true;
}

WorldPhysics::WorldPhysics()
{
    createPhysics();
}

WorldPhysics::~WorldPhysics()
{
    deletePhysics();
}

void WorldPhysics::deletePhysics()
{
	if(dynamicsWorld)
	{
		for (int i=bodies.size()-1; i>=0; i--)
		{
			bodies[i]->removeFromWorld();
		}
		bodies.clear();
		delete dynamicsWorld;
	}
	if(solver)					delete solver;
	if(collisionConfiguration)	delete collisionConfiguration;
	if(dispatcher)				delete dispatcher;
	if(broadphase)				delete broadphase;
}

void WorldPhysics::createPhysics()
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

	gContactProcessedCallback = CustomProcessedCallback;
}

void WorldPhysics::Clear()
{
    deletePhysics();
    createPhysics();
}

void WorldPhysics::update()
{
    dynamicsWorld->stepSimulation(egg::getInstance().g_Delta,1);
}

SolidBody* WorldPhysics::addBody(float mass, float radius)
{
	std::shared_ptr<SolidBody> newBody = std::make_shared<SolidBody>(mass, radius);
	if(!newBody || !dynamicsWorld) return nullptr;

	newBody->setWorld(dynamicsWorld);
	newBody->addToWorld();
	bodies.push_back(newBody);
	return newBody.get();
}

SolidBody* WorldPhysics::addBody(float mass, glm::vec3 boxHalfExtents)
{
	std::shared_ptr<SolidBody> newBody = std::make_shared<SolidBody>(mass, boxHalfExtents);
	if(!newBody || !dynamicsWorld) return nullptr;

	newBody->setWorld(dynamicsWorld);
	newBody->addToWorld();
	bodies.push_back(newBody);
	return newBody.get();
}

SolidBody* WorldPhysics::addBody(float mass, Mesh* mesh)
{
	std::shared_ptr<SolidBody> newBody = std::make_shared<SolidBody>(mass, mesh);
	if(!newBody || !dynamicsWorld) return nullptr;

	newBody->setWorld(dynamicsWorld);
	newBody->addToWorld();
	bodies.push_back(newBody);
	return newBody.get();
}

void WorldPhysics::remBody(SolidBody*& body)
{
	if(body == nullptr) return;
	for(int i = bodies.size()-1; i>=0; i--)
	{
		if(bodies[i].get() == body)
		{
			body->removeFromWorld();
			bodies.erase(bodies.begin() + i);
			body = nullptr;
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

