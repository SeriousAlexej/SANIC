#ifndef REGISTEREVENTS_H
#define REGISTEREVENTS_H

#include "entity.h"
#include "entityevent.h"
#include <sstream>
#include <memory>

void registerEvents(Lua& lua)
{
    auto newEvent = lua.CreateFunction<LuaUserdata<EntityEvent>(std::string)>(
    [&](std::string type) -> LuaUserdata<EntityEvent>
    {
        if(type == "Damage")
        {
            EventDamage* ped = new EventDamage(nullptr, 0);
            LuaUserdata<EntityEvent> lud = lua.CreateUserdata<EntityEvent>(ped);
                            
            lud.Set("getEventcode",
                lua.CreateFunction<int()>([&] { return ped->eventCode; })
            );
            lud.Set("getAmount", LUA_GETTER(lua, int, ped, amount));
            lud.Set("getDamager", LUA_UGETTER(lua, Entity, ped, penDamager));
            lud.Set("setAmount", LUA_SETTER(lua, int, ped, amount));
            lud.Set("setDamager", LUA_USETTER(lua, Entity, ped, penDamager));
            
            return lud;
        }
        if(type == "Begin")
		{
			EventBegin* peb = new EventBegin();
			LuaUserdata<EntityEvent> lud = lua.CreateUserdata<EntityEvent>(peb);
            
            return lud;
		}
		else
		{
			EntityEvent* pev = new EntityEvent();
                        std::stringstream(type) >> pev->eventCode;
			LuaUserdata<EntityEvent> lud = lua.CreateUserdata<EntityEvent>(pev);
            
            return lud;
		}
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

