#ifndef REGISTEREVENTS_H
#define REGISTEREVENTS_H

#include "entity.h"
#include "entityevent.h"
#include <sstream>

#define LUA_GETTER(x, y, z) \
lua.CreateFunction<x()>([&, y]() { \
    return y->z; \
})

#define LUA_SETTER(x, y, z) \
lua.CreateFunction<void(x)>([&, y](x a) { \
    y->z = a; \
})

#define LUA_UGETTER(x, y, z) \
lua.CreateFunction<LuaUserdata<x>()>([&, y]() { \
    auto eud = lua.CreateUserdata(y->z); \
    if(y->z != nullptr) y->z->registerLua(eud); \
    return eud; \
})

#define LUA_USETTER(x, y, z) \
lua.CreateFunction<void(LuaUserdata<x>)>([&, y](LuaUserdata<x> a) { \
    y->z = a.GetPointer(); \
})

void registerEvents(Lua& lua)
{
    auto newEvent = lua.CreateFunction<LuaUserdata<EntityEvent>(std::string)>(
    [&](std::string type) -> LuaUserdata<EntityEvent>
    {
		LuaUserdata<EntityEvent> *lud;
        if(type == "Damage")
        {
            EventDamage* ped = new EventDamage(nullptr, 0);
            *lud = lua.CreateUserdata<EntityEvent>(ped);
            lud->Set("getAmount", LUA_GETTER(int, ped, amount));
            lud->Set("getDamager", LUA_UGETTER(Entity, ped, penDamager));
            lud->Set("setAmount", LUA_SETTER(int, ped, amount));
            lud->Set("setDamager", LUA_USETTER(Entity, ped, penDamager));
        }
        if(type == "Begin")
		{
			EventBegin* peb = new EventBegin();
			*lud = lua.CreateUserdata<EntityEvent>(peb);
		}
		else
		{
			EntityEvent* pev = new EntityEvent();
			stringstream(type) >> pev->eventCode;
			*lud = lua.CreateUserdata<EntityEvent>(pev);
		}
		auto nlud = *lud;
		delete lud;
		return nlud;
    });
    auto dtable = lua.CreateTable();
    dtable.Set("new", newEvent);
    lua.GetGlobalEnvironment().Set("EntityEvent", dtable);
}

//void registerStates(Lua& lua)
//{
//	auto newstate = lua.CreateFunction<LuaFunction<void(LuaUserdata<EntityEvent>, LuaUserdata<Entity>)>>(
//		[&]() -> LuaFunction<LuaUserdata<EntityEvent>, LuaUserdata<Entity>>
//		{
//
//		});
//}

/* ???
#undef LUA_GETTER
#undef LUA_SETTER
#undef LUA_UGETTER
#undef LUA_USETTER
*/

#endif // REGISTEREVENTS_H

