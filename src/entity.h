#ifndef _ENTITY_H_
#define _ENTITY_H_
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <luacppinterface.h>
#include "global.h"
#include "basic.h"
#include "properties.h"
#include "entitypointer.h"
#include "entities/incubator.h"

class Event;
class Entity;
class EntityPointer;
class EntityEvent;
class SolidBody;
class ModelSet;
class WorldGraphics;
class WorldPhysics;
class World;

typedef LuaFunction<void(LuaUserdata<EntityEvent>, LuaUserdata<Entity>)> luaCallbackFunction;
/*
class LuaCallback : public stateCallback // TODO: you know this should be less lame
{
public:
    LuaCallback(luaCallbackFunction _f) : func(_f) {}
    void operator()(EntityEvent* ee, Entity* e) {
        LuaUserdata<EntityEvent> eeud = egg::getInstance().g_lua.CreateUserdata(ee);
        LuaUserdata<Entity> eud = egg::getInstance().g_lua.CreateUserdata(e);
        func.Invoke(eeud, eud);
    }
private:
    luaCallbackFunction func;
};
*/

#include "entityAImacros.inc"

class Entity : public Touchable, public Unique, public FamilyTree, public Serial
{
public:
    typedef std::unordered_map<std::string, Property> Props;
    typedef std::unique_ptr<Event> EventPtr;

#include "entityState.inc"

    Entity();
    virtual ~Entity();

    template<class T> const T&  getProperty(const std::string& name);
    template<class T> void      setProperty(const std::string& s, const T& val);
    void                        sendEvent(EventPtr e);

    virtual void                initialize();
    virtual void                setupAI();

    void                        destroy();

    void                        switchToEditorModel();
    void                        switchToModel();

    virtual void                Deserialize(rapidjson::Value& d);
    virtual rapidjson::Value    Serialize(rapidjson::Document& d);
    virtual rapidjson::Value    SerializeForCopying(rapidjson::Document& d);

    const SolidBody*            getBody() const { return body; }
    const ModelSet*             getModelSet() const { return modelset; }

    const inline bool           isTouchable() const { return touchable; }

    void                        setupCollision(float mass);
    void                        setupCollision(float mass, float &radius);
    void                        setupCollision(float mass, glm::vec3 &halfExtents);

    const EntityPointer&        getParent() const;

    void                        setPosition(glm::vec3 pos);
    void                        setRotation(glm::vec3 rot);
    void                        setRotation(glm::quat rot);

    void                        pointerAdded(EntityPointer* pen);
    void                        pointerLeft(EntityPointer* pen);

    virtual void                registerLua(LuaUserdata<Entity>& lua);
    LuaUserdata<Entity>     private_lud; // FIXME

    Props&                      getProperties() { return properties; }

    void                        setSelected(bool s);

protected:
    typedef std::unordered_map<std::string, EntityState> States;
    enum StateChange {
        SC_NOCHANGE,
        SC_PUSH,
        SC_SWAP,
        SC_POP
    };

    void                        update();
    void                        updateAI();
    virtual void                updatePosition();
    void                        parentMoved();
    void                        setupModel();
    void                        setupParent();
    bool                        childrenContain(const Entity* e) const;

    void                        setTimer(float wTime);
    bool                        pushState(EntityState* es);
    bool                        popState();
    bool                        swapState(EntityState* es);

    bool                        selected; //editor only
    bool                        background;
    bool                        hasCollision;
    bool                        editorOnly;
    SolidBody*                  body;
    ModelSet*                   modelset;
    Props                       properties;
    std::string                 name;
    FileName                    modelPath;
    bool                        touchable;
    float                       mass;
    glm::vec3                   rotationEuler;
    glm::vec3                   position;

    std::vector<EntityState*>   stateStack;
    std::vector<EventPtr>       events;
    std::vector<EventPtr>       eventsNextTick;
    StateChange                 stateChange = SC_NOCHANGE;
    EntityState*                nextState = nullptr;
    float                       timerSeconds = 0.0f;
    unsigned                    _phase = 0;
    float                       _waitTime = 0.0f;
    States                      states;

    std::vector<EntityPointer*> pointAtMe;
    EntityPointer               parent;
    EntityPointer               parentOld;
    std::vector<Entity*>        children;
    glm::mat4                   relativeTransform;
    glm::mat4                   oldMatrix;

    WorldGraphics*              wldGFX; //fill theese
    WorldPhysics*               wldPHY; //from class World
    World*                      wld;

    friend class World;

    template<class C>
    void registerProperties(const std::string& s, C* c, std::function<void(void)> cb);

    template<class C, class... T>
    void registerProperties(const std::string& s, C* c, std::function<void(void)> cb, T... Args);

public:
    template<class C>
    typename std::enable_if<std::is_base_of<FromLua, C>::value>::type
         addToLua(LuaUserdata<Entity>& l, const std::string& s, C c);

    template<class C>
    typename std::enable_if<!std::is_base_of<FromLua, C>::value>::type
         addToLua(LuaUserdata<Entity>& l, const std::string& s, C c);

    template<class C, class... T>
    void addToLua(LuaUserdata<Entity> &l, const std::string& s, C c, T... args);
};

#include "entityTemplates.inc"

#endif
