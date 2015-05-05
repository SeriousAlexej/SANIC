#include "solidbody.h"
#include "default_model.h"

SolidBody::SolidBody(float _mass, float radius)
{
    offset = btVector3(0,0,0);
	world = NULL;
	//removeMe = false;
	indexVertexArrays = NULL;
	trimeshShape = NULL;
	if(radius < 0.01f)
	{
		printf("Someone set radius to %f, too small, changing to 1.0...\n", radius);
		radius = 1.0f;
	}
	collShape = new btSphereShape(btScalar(radius));
	collisionType = CollSphere;
	createBodyFromShape(_mass);
}

SolidBody::SolidBody(float _mass, glm::vec3 boxHalfExtents)
{
    offset = btVector3(0,0,0);
	world = NULL;
	//removeMe = false;
	indexVertexArrays = NULL;
	trimeshShape = NULL;
	for(int i=0; i<3; i++)
	{
		if(boxHalfExtents[i] <= 0.01f)
		{
			printf("Someone set box dimension %d to %f, too small, changing to 1.0...\n", i, boxHalfExtents[i]);
			boxHalfExtents[i] = 1.0f;
		}
	}
	collShape = new btBoxShape(btVector3(boxHalfExtents.x, boxHalfExtents.y, boxHalfExtents.z));
	collisionType = CollBox;
	createBodyFromShape(_mass);
}

SolidBody::SolidBody(float _mass, ModelInstance *mi)
{
    offset = btVector3(0,0,0);
	world = NULL;
	//removeMe = false;
	assert(mi != NULL);
	Mesh* mesh = mi->pMesh;
	assert(mesh != NULL);

	if(!mesh->isOk)
	{
		collShape = new btBoxShape(btVector3(0.5, 0.5, 0.5));
		collisionType = CollBox;
		createBodyFromShape(_mass);
		return;
	}

	indexVertexArrays = new btTriangleIndexVertexArray(
		mesh->indices.size()/3,
		&mesh->indices[0],
		3*sizeof(int),
		mesh->frames[0].vertices.size(),
		&mesh->frames[0].vertices[0][0],
		sizeof(glm::vec3));

	btVector3 aabbMin(-1000,-1000,-1000),aabbMax(1000,1000,1000);
	trimeshShape  = new btBvhTriangleMeshShape(indexVertexArrays,true,aabbMin,aabbMax);
	collShape = trimeshShape;
	btTriangleInfoMap* triangleInfoMap = new btTriangleInfoMap();
	btGenerateInternalEdgeInfo(trimeshShape,triangleInfoMap);

	collisionType = CollMesh;
	createBodyFromShape(_mass);
}

SolidBody::~SolidBody()
{
	if(compoundShape)       delete compoundShape;
	if(collisionType == CollMesh)
	{
		if(trimeshShape)	delete trimeshShape;
	} else {
		if(collShape)		delete collShape;
	}
	if(rigidBody)			delete rigidBody;
	if(indexVertexArrays)	delete indexVertexArrays;
}

void SolidBody::activate()
{
	if(rigidBody)
	{
        if(world)
        {
            world->removeRigidBody(rigidBody);
            rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() & ~(btCollisionObject::CF_NO_CONTACT_RESPONSE));
            btVector3 in;
            compoundShape->calculateLocalInertia(mass, in);
            rigidBody->setMassProps(mass, in);
            rigidBody->updateInertiaTensor();
            world->addRigidBody(rigidBody);
        }
	}
}

void SolidBody::deactivate()
{
	if(rigidBody)
	{
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
		rigidBody->setMassProps(0, btVector3(0,0,0));
		this->setVelocity(glm::vec3(0,0,0));
		this->setAngularVelocity(glm::vec3(0,0,0));
	}
}

void SolidBody::createBodyFromShape(btScalar _mass)
{
	compoundShape = new btCompoundShape();
	btTransform localTrans;
	localTrans.setIdentity();
	localTrans.setOrigin(offset);
	compoundShape->addChildShape(localTrans, (collShape==NULL?trimeshShape:collShape));
	mass = _mass;
	btTransform startTransform;
	startTransform.setIdentity();
	btVector3 localInertia(0,0,0);
	if(collisionType != CollMesh) //CollMesh doesn't support that
	{
		compoundShape->calculateLocalInertia(mass,localInertia);
	}
	btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,compoundShape,localInertia);

	rbInfo.m_friction = 1.0;
	rbInfo.m_rollingFriction = 0.5;
	rbInfo.m_angularDamping = 0.1;

	rigidBody = new btRigidBody(rbInfo);
	rigidBody->setActivationState(DISABLE_DEACTIVATION);
}

void SolidBody::addToWorld()
{
	if(world && rigidBody)
		world->addRigidBody(rigidBody);
}

void SolidBody::removeFromWorld()
{
	if(world && rigidBody)
	{
		world->removeCollisionObject(rigidBody);
		if(rigidBody->getMotionState())
		{
			delete rigidBody->getMotionState();
		}
	}
	delete this;
}

void SolidBody::setPosition(glm::vec3 pos)
{
	if(!rigidBody) return;
	btTransform t = rigidBody->getWorldTransform();
	t.setOrigin(btVector3(pos.x, pos.y, pos.z));
	rigidBody->setWorldTransform(t);
}

void SolidBody::stopMoving()
{
	this->setVelocity(glm::vec3(0,0,0));
}

void SolidBody::stopRotating()
{
	this->setAngularVelocity(glm::vec3(0,0,0));
}

void SolidBody::addVelocity(glm::vec3 vel)
{
	if(!rigidBody) return;
	rigidBody->setLinearVelocity(rigidBody->getLinearVelocity() + btVector3(vel.x, vel.y, vel.z));
}

void SolidBody::addAngularVelocity(glm::vec3 vel)
{
	if(!rigidBody) return;
	rigidBody->setAngularVelocity(rigidBody->getAngularVelocity() + btVector3(vel.x, vel.y, vel.z));
}

void SolidBody::setVelocity(glm::vec3 vel)
{
	if(!rigidBody) return;
	rigidBody->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
}

void SolidBody::setAngularVelocity(glm::vec3 vel)
{
	if(!rigidBody) return;
	rigidBody->setAngularVelocity(btVector3(vel.x, vel.y, vel.z));
}

glm::vec3 SolidBody::getVelocity()
{
	if(!rigidBody) return glm::vec3(0,0,0);
	btVector3 vel = rigidBody->getLinearVelocity();
	return glm::vec3(vel.getX(), vel.getY(), vel.getZ());
}

glm::vec3 SolidBody::getAngularVelocity()
{
	if(!rigidBody) return glm::vec3(0,0,0);
	btVector3 vel = rigidBody->getAngularVelocity();
	return glm::vec3(vel.getX(), vel.getY(), vel.getZ());
}

void SolidBody::setRotation(glm::vec3 rot)
{
	if(!rigidBody) return;
	btQuaternion q;
	q.setEulerZYX(rot.z, rot.y, rot.x);
	btTransform t = rigidBody->getWorldTransform();
	t.setRotation(q);
	rigidBody->setWorldTransform(t);
}

void SolidBody::setRotation(float angle, glm::vec3 dir)
{
	if(!rigidBody) return;
	btQuaternion q;
	q.setRotation(btVector3(dir.x, dir.y, dir.z), glm::radians(angle));
	btTransform t = rigidBody->getWorldTransform();
	t.setRotation(q);
	rigidBody->setWorldTransform(t);
}

void SolidBody::rotate(glm::vec3 rot, bool absolute)
{
	if(!rigidBody) return;
	btQuaternion q;
	q.setEulerZYX(rot.z, rot.y, rot.x);
	btQuaternion q_old = rigidBody->getOrientation();
	btTransform t = rigidBody->getWorldTransform();
	if(absolute)
	{
		t.setRotation(q * q_old);
	} else {
		t.setRotation(q_old * q);
	}
	rigidBody->setWorldTransform(t);
}

void SolidBody::translate(glm::vec3 pos)
{
	if(!rigidBody) return;
	btTransform t = rigidBody->getWorldTransform();
	t.setOrigin(t.getOrigin() + btVector3(pos.x, pos.y, pos.z));
	rigidBody->setWorldTransform(t);
}

void SolidBody::setOffset(glm::vec3 off)
{
    offset = btVector3(off.x, off.y, off.z);
    if(!rigidBody || !compoundShape) return;
    btTransform trans;
    trans.setIdentity();
    trans.setOrigin(offset);
    compoundShape->updateChildTransform(0, trans);
}

glm::vec3 SolidBody::getOffset() const
{
    return glm::vec3(offset.getX(), offset.getY(), offset.getZ());
}

glm::vec3 SolidBody::getPosition()
{
	if(!rigidBody) return glm::vec3(0,0,0);
	btVector3 pos = rigidBody->getWorldTransform().getOrigin();
	glm::vec3 position(pos.getX(), pos.getY(), pos.getZ());
	return position;
}

glm::vec3 SolidBody::getRotation()
{
	if(!rigidBody) return glm::vec3(0,0,0);
	glm::vec3 rot;
	btMatrix3x3 mat(rigidBody->getOrientation());
	mat.getEulerZYX(rot.z, rot.y, rot.x);
	return rot;
}

glm::quat SolidBody::getRotationQuat()
{
	if(!rigidBody) return glm::quat(0,0,0,0);
	btQuaternion q = rigidBody->getOrientation();
	return glm::quat(q.getW(), q.getX(), q.getY(), q.getZ());
}

void SolidBody::teleport(glm::vec3 pos, glm::vec3 rot)
{
	if(!rigidBody) return;
	btQuaternion q;
	q.setEulerZYX(rot.z, rot.y, rot.x);
	btTransform trans;
	trans.setIdentity();
	trans.setOrigin(btVector3(pos.x, pos.y, pos.z));
	trans.setRotation(q);
	rigidBody->setWorldTransform(trans);
}

void SolidBody::setWorld(btDiscreteDynamicsWorld *myworld)
{
	assert(myworld);
	world = myworld;
}

void SolidBody::setOwner(void *owner)
{
	if(rigidBody)
		rigidBody->setUserPointer(owner);
	if(compoundShape)
		compoundShape->setUserPointer(owner);
}

void* SolidBody::getOwner()
{
	if(rigidBody)
	{
		if(rigidBody->getUserPointer() != NULL)
			return rigidBody->getUserPointer();
	} else
	if(compoundShape)
	{
		return compoundShape->getUserPointer();
	}
	return NULL;
}

void SolidBody::enableTouching()
{
	if(rigidBody)
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
}
