#include "world.h"
#include "entities/incubator.h"
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>

//these vars assist EntityPointer deserialization
std::map<int, Entity*> enByOldId; //get entity by ID it was saved with
std::map<EntityPointer*, Entity*> pensOwner; //get Entity that owns given EntityPointer
std::vector<EntityPointer*> pensToRetarget; //all EntityPointers that need retargeting
Entity* beingDeserialized = nullptr; //current entity being deserialized, used to fill pensOwner

World::World()
{
    // lua.GetGlobalEnvironment();
    // lua.GetRegistry();
    //penGraphics = nullptr;
    lua = new Lua();
    lua->LoadStandardLibraries();
    pGraphics = new WorldGraphics();
    for(auto kv : Incubator::getInstance().cookBook)
    {
        registerEntity(kv.first);
    }
    registerEvents(*lua);
}

void World::deleteAllEntities()
{
	for(int i=entities.size()-1; i>=0; i--)
	{
		//delete entities[i];
        void* ptr = (dynamic_cast<FromIncubator*>(entities[i]))->ptr;
        entities[i]->~Entity();
        ::operator delete(ptr);
	}
	entities.clear();
}

World::~World()
{
    deleteAllEntities();
    delete pGraphics;
}

void World::Clear()
{
    deleteAllEntities();
	if(pGraphics != nullptr)
    {
        delete pGraphics;
    }
    physics.Clear();
    pGraphics = new WorldGraphics();
}

void World::registerEntity(const std::string& name)
{
    auto entityComputer = lua->CreateFunction<LuaUserdata<Entity>() >(
    [&, name]() -> LuaUserdata<Entity>
    {
        Entity* pen = createEntity(name);
        auto entity = lua->CreateUserdata<Entity>(pen);
        pen->registerLua(entity);
        return entity;
    });

    auto entityType = lua->CreateTable();
    entityType.Set("new", entityComputer);
    lua->GetGlobalEnvironment().Set(name.c_str(), entityType);
}

void World::update()
{
	if(!g_Editor)
	{
		for(int i=entities.size()-1; i>=0; i--)
		{
			entities[i]->update();
		}
		physics.update();
    }
}


Entity* World::createEntity(const std::string& entityName)
{
    return createEntity(static_cast<Entity*>(Incubator::Create(entityName)));
}

Entity* World::createEntity(Entity* e)
{
	//can't add already added or null entity
    if(!(e != nullptr && e->wldGFX == nullptr && e->wldPHY == nullptr && e->wld == nullptr))
    {
        //std::cout << "Vse ploxo" << std::endl;
        throw cant_create();
    }

    e->wldGFX = pGraphics;
	e->wldPHY = &physics;
	e->wld = this;
    e->pLua = lua;
	e->initialize();
	entities.push_back(e);
	if(pGraphics != nullptr)
    {//useful in editor. If called from Load, then position will be reset anyway, so no harm done.
        e->setPosition(pGraphics->camera.getPosition() + pGraphics->camera.getFront()*3.0f);
    }
	return e;
}

void World::removeEntity(Entity *e)
{
	if(e)
	{
		for(int i=entities.size()-1; i>=0; i--)
		{
			if(entities[i]==e)
			{
				//delete e;
				void* ptr = (dynamic_cast<FromIncubator*>(e))->ptr;
				e->~Entity();
				::operator delete(ptr);
				entities.erase(entities.begin() + i);
				return;
			}
		}
	}
}

RayCastInfo World::castRay(glm::vec3 origin, glm::vec3 direction)
{
	RayCastInfo rci;
	rci.enHit = NULL;
	rci.normHit = glm::vec3(0,0,0);
	rci.posHit = glm::vec3(0,0,0);
	rci.posOrigin = origin;
	rci.direction = glm::normalize(direction);
	rci.rayLength = 0.0f;

	direction = glm::normalize(direction)*1000.0f;

	btCollisionWorld::ClosestRayResultCallback RayCallback(
		btVector3(origin.x, origin.y, origin.z),
		btVector3(direction.x, direction.y, direction.z)
		);
	physics.dynamicsWorld->rayTest(
		btVector3(origin.x, origin.y, origin.z),
		btVector3(direction.x, direction.y, direction.z),
		RayCallback
		);

	if(RayCallback.hasHit() && RayCallback.m_collisionObject->getUserPointer() != nullptr)
	{
		rci.enHit = static_cast<Entity*>(RayCallback.m_collisionObject->getUserPointer());
		btVector3 &hw = RayCallback.m_hitPointWorld;
		btVector3 &nw = RayCallback.m_hitNormalWorld;
		rci.normHit = glm::vec3(nw.getX(), nw.getY(), nw.getZ());
		rci.posHit = glm::vec3(hw.getX(), hw.getY(), hw.getZ());
		rci.rayLength = glm::distance(rci.posHit, rci.posOrigin);
	}

	return rci;
}

void World::Save(const string &filename)
{
    FILE* fp = fopen(filename.c_str(), "w");
    rapidjson::Document doc;
    rapidjson::Value out;
    out.SetArray();
    for(auto en : entities)
    {
        Entity& penEntity = *en;
        rapidjson::Value val = penEntity.Serialize(doc);

        rapidjson::Value classname;
        std::string strClass = penEntity.getClass();
        classname.SetString(strClass.c_str(), strClass.length(), doc.GetAllocator());
        val.AddMember("class", classname, doc.GetAllocator());

        rapidjson::Value id;
        id.SetInt(penEntity.getMultipass());
        val.AddMember("id", id, doc.GetAllocator());

        out.PushBack(val, doc.GetAllocator());
    }
    char writeBuffer[65536];
    rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
    out.Accept(writer);
    fclose(fp);
}

void World::Love(const string &filename)
{
    Clear();

    beingDeserialized = nullptr;
    pensOwner.clear();
    enByOldId.clear();
    pensToRetarget.clear();

    FILE* fp = fopen(filename.c_str(), "r");
    char readBuffer[65536];
    rapidjson::Document doc;
    rapidjson::FileReadStream str(fp, readBuffer, sizeof(readBuffer));

    doc.ParseStream(str);
    for(auto it = doc.Begin(); it != doc.End(); ++it)
    {
        std::string classname = (*it)["class"].GetString();
        Entity* pen = createEntity(classname);
        beingDeserialized = pen;
        pen->Deserialize(*it);
    }

    for(EntityPointer* p : pensToRetarget)
    {
        if(p->Name() == "Parent")
        {
            pensOwner[p]->setParent(enByOldId[p->GetCurrentID()]);
        } else {
            *p = enByOldId[p->GetCurrentID()];
        }
    }

    beingDeserialized = nullptr;
    pensOwner.clear();
    enByOldId.clear();
    pensToRetarget.clear();

    fclose(fp);
}
