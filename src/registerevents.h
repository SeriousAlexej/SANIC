#ifndef REGISTEREVENTS_H
#define REGISTEREVENTS_H

#include "entity.h"
#include "entityevent.h"

void registerEvents(Lua& lua)
{
    auto newEvent = lua.CreateFunction<LuaUserdata<EntityEvent>(std::string)>(
    [&](std::string type) -> LuaUserdata<EntityEvent>
    {
        if(type == "Damage")
        {
            EventDamage* ped = new EventDamage(nullptr, 0);
            auto lud = lua.CreateUserdata<EntityEvent>(new EventDamage(nullptr, 0));
            lud.Set("getAmount", lua.CreateFunction<int()>([&, ped]() {
                return ped->amount;
            }));
            lud.Set("getDamager", lua.CreateFunction<LuaUserdata<Entity>()>([&, ped]() {
                auto eud = lua.CreateUserdata(ped->penDamager);
                if(ped->penDamager != nullptr) {
                    ped->penDamager->registerLua(eud);
                }
                return eud;
            }));
            lud.Set("setAmount", lua.CreateFunction<void(int)>([&, ped](int a) {
                ped->amount = a;
            }));
            lud.Set("setDamager", lua.CreateFunction<void(LuaUserdata<Entity>)>([&, ped](LuaUserdata<Entity> eud) {
                ped->penDamager = eud.GetPointer();
            }));
            return lud;
        }
    });
    auto dtable = lua.CreateTable();
    dtable.Set("new", newEvent);
    lua.GetGlobalEnvironment().Set("EntityEvent", dtable);
}

#endif // REGISTEREVENTS_H

