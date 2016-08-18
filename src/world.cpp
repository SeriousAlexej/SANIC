#include "world.h"
#include "entities/incubator.h"
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/prettywriter.h>
#include <SFML/Window/Keyboard.hpp>
#include "global.h"
#include "solidbody.h"
#include "entity.h"

World::World()
{
    // lua.GetGlobalEnvironment();
    // lua.GetRegistry();
    //penGraphics = nullptr;
    pGraphics = new WorldGraphics();
    registerLua();
}

void World::registerLua()
{
    for(auto kv : Incubator::getInstance().cookBook)
    {
        registerEntity(kv.first);
    }
    auto getEntitiesList = egg::getInstance().g_lua.CreateFunction<LuaTable()>([&]() {
        LuaTable result = egg::getInstance().g_lua.CreateTable();
        for(Entity* pen : entities) {
            result.Set<LuaUserdata<Entity>>(pen->getMultipass(), pen->private_lud);
        }
        return result;
    });
    auto table = egg::getInstance().g_lua.CreateTable();
    table.Set("getEntities", getEntitiesList);
    egg::getInstance().g_lua.GetGlobalEnvironment().Set("World", table);
}

void World::deleteAllEntities()
{
	entities.clear();
	obsoleteEntitties.clear();
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
    auto entityComputer = egg::getInstance().g_lua.CreateFunction<LuaUserdata<Entity>() >(
    [&, name]() -> LuaUserdata<Entity>
    {
        Entity* pen = createEntity(name);
        return pen->private_lud;
    });

    auto entityType = egg::getInstance().g_lua.CreateTable();
    entityType.Set("new", entityComputer);
    egg::getInstance().g_lua.GetGlobalEnvironment().Set(name.c_str(), entityType);
}

void World::update()
{
    obsoleteEntitties.clear();

    for(int i=entities.size()-1; i>=0; i--)
    {
        entities[i]->update();
        if(!egg::getInstance().g_Editor)
        {
            entities[i]->updateAI();
            physics.update();
        }
    }
    if(pGraphics)
    {
        pGraphics->render();
    }
}

Entity* World::createEntity(const std::string& entityName)
{
    return createEntity(Incubator::Create(entityName));
}

Entity* World::createEntity(std::shared_ptr<Entity> e)
{
	//can't add already added or null entity
    if(!(e.get() != nullptr && e->wldGFX == nullptr && e->wldPHY == nullptr && e->wld == nullptr))
    {
        throw cant_create();
    }

    e->wldGFX = pGraphics;
	e->wldPHY = &physics;
	e->wld = this;
	e->initialize();
	e->setupAI();
	entities.push_back(e);
	if(pGraphics && egg::getInstance().g_Editor)
    {//useful in editor. If called from Load, then position will be reset anyway, so no harm done.
        e->setPosition(pGraphics->camera.getPosition() + pGraphics->camera.getFront()*10.0f);
    }
	return e.get();
}

Entity* World::GetEntityWithID(int id)
{
    for(int i=entities.size()-1; i>=0; i--)
    {
        if(entities[i]->getMultipass() == id)
            return entities[i].get();
    }
    return nullptr;
}

void World::removeEntity(Entity *e)
{
	if(e)
	{
		for(int i=entities.size()-1; i>=0; i--)
		{
			if(entities[i].get()==e)
			{
				obsoleteEntitties.push_back(entities[i]);
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

void World::Save(const std::string &filename)
{
    FILE* fp = fopen(filename.c_str(), "w");
    rapidjson::Document doc;
    rapidjson::Value out;
    out.SetArray();
    for(auto &en : entities)
    {
        Entity& penEntity = *(en.get());
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
    rapidjson::PrettyWriter<rapidjson::FileWriteStream, rapidjson::Document::EncodingType, rapidjson::UTF8<> > writer(os);
    out.Accept(writer);
    fclose(fp);
}

void World::Love(const std::string &filename)
{
    Clear();

    FILE* fp = fopen(filename.c_str(), "r");
    char readBuffer[65536];
    rapidjson::Document doc;
    rapidjson::FileReadStream str(fp, readBuffer, sizeof(readBuffer));

    doc.ParseStream(str);
    for(auto it = doc.Begin(); it != doc.End(); ++it)
    {
        std::string classname = (*it)["class"].GetString();
        Entity* pen = createEntity(classname);
        pen->setMultipass((*it)["id"].GetInt());
        pen->properties["Position"].Deserialize((*it)["Position"]);
        pen->properties["Rotation"].Deserialize((*it)["Rotation"]);
    }
    int i=0;
    for(auto it = doc.Begin(); it != doc.End(); ++it)
    {
        assert(entities[i]->getMultipass() == (*it)["id"].GetInt());
        entities[i++]->Deserialize(*it);
    }

    fclose(fp);

    if(pGraphics != nullptr)
    {
        pGraphics->sortForBackground();
    }
}

Entity* World::Paste(std::string& src)
{
    rapidjson::Document doc;
    doc.Parse(src.c_str());
    auto it = doc.Begin();
    std::string classname = (*it)["class"].GetString();
    Entity* pen = createEntity(classname);
    pen->Deserialize(*it);
    return pen;
}
