#ifndef _ENTITY_EVENT_H_
#define _ENTITY_EVENT_H_
#include <luacppinterface.h>
#include <memory>
#include "entity.h"

class Entity;

using std::make_shared;

/*********************************************************************/

const int EventCode_Dummy = 0;
class EntityEvent
{
public:
	EntityEvent() : eventCode(EventCode_Dummy) {}
	virtual ~EntityEvent() {}
	unsigned int	eventCode;
};

/*********************************************************************/

const int EventCode_Begin = 1;
class EventBegin : public EntityEvent
{
public:
	EventBegin()
	{
		eventCode = EventCode_Begin;
	}
	virtual ~EventBegin() {}
};

/*********************************************************************/

const int EventCode_Touch = 2;
class EventTouch : public EntityEvent
{
public:
	EventTouch(void* _toucher)
	{
		eventCode = EventCode_Touch;
		assert(_toucher != NULL);
		toucher = _toucher;
	}
	virtual ~EventTouch() {}
	void*	toucher;
};

/*********************************************************************/

const int EventCode_Timer = 3;
class EventTimer : public EntityEvent
{
public:
	EventTimer()
	{
		eventCode = EventCode_Timer;
	}
	virtual ~EventTimer() {}
};

/*********************************************************************/

const int EventCode_AutowaitCallback = 4;
class EventAutowaitCallback : public EntityEvent
{
public:
	EventAutowaitCallback(int i)
	{
		eventCode = EventCode_AutowaitCallback;
		index = i;
	}
	virtual ~EventAutowaitCallback() {}
	int	index;
};

/*********************************************************************/

const int EventCode_Damage = 5;
class EventDamage : public EntityEvent
{
public:
    Entity* penDamager;
    int amount;
    EventDamage(Entity* pen, int a) : penDamager(pen), amount(a) {}
};

static void registerEvents(Lua& lua)
{
    auto newEvent = lua.CreateFunction<LuaUserdata<EntityEvent>(std::string)>(
    [&](std::string type) -> LuaUserdata<EntityEvent>
    {
        if(type == "Damage")
        {
            EventDamage* ped = new EventDamage(nullptr, 0);
            auto lud = lua.CreateUserdata<EntityEvent>(ped);
            lud.Set("getAmount", genLuaGetter(lua, &(ped->amount)));
            lud.Set("getDamager", genUserdataGetter<Entity>(lua, (ped->penDamager)));
            lud.Set("setAmount", genLuaSetter(lua, &(ped->amount)));
            lud.Set("setDamager", genUserdataSetter<Entity>(lua, (ped->penDamager)));
            return lud;
        }
    });
    auto dtable = lua.CreateTable();
    dtable.Set("new", newEvent);
    lua.GetGlobalEnvironment().Set("EntityEvent", dtable);
}

/*********************************************************************/


#endif