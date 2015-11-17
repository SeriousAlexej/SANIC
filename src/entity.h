#ifndef _ENTITY_H_
#define _ENTITY_H_
#include <list>
#include <stack>
#include <string>
#include <map>
#include <functional>
#include <initializer_list>
#include <AntTweakBar.h>
#include "basic.h"
#include "solidbody.h"
#include "modelset.h"
#include "properties.h"
#include "entitypointer.h"
#include "entityevent.h"
#include "entities/incubator.h"
#include <luacppinterface.h>
#include <type_traits>
#include "global.h"

class Entity;
class EntityPointer;
class EntityEvent;
class WorldGraphics;
class WorldPhysics;
class World;

typedef std::function<void(EntityEvent*,Entity*)> stateCallback;
typedef LuaFunction<void(LuaUserdata<EntityEvent>, LuaUserdata<Entity>)> luaCallbackFunction;

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

#define IMPLEMENT_STATE(c, s) \
void c::s(EntityEvent* ee, Entity* cl) { c *caller = dynamic_cast<c *>(cl);
#define DECLARE_STATE(x) \
static void x(EntityEvent* ee, Entity* caller)
#define nextwait(t) \
caller->_autowait(t, __LINE__); return; } case __LINE__: {
#define autowait(t, i) \
caller->_autowait(t, i); return;
#define switchEvent \
switch(ee->eventCode)
    #define switchAutowait \
    EventAutowaitCallback *eac = dynamic_cast<EventAutowaitCallback*>(ee); switch(eac->index)
#define END_STATE() \
    caller->popState(); }

/**
 * @brief Defines gui control for entity prop
 */
struct DrawableElement
{
    enum ElemType
    {
        //variable types
        PT_BOOL     = TW_TYPE_BOOLCPP,
        PT_INT      = TW_TYPE_INT32,
        PT_FLOAT    = TW_TYPE_FLOAT,
        PT_COLOR    = TW_TYPE_COLOR3F,
        PT_STRING   = TW_TYPE_STDSTRING,
        PT_QUAT     = TW_TYPE_QUAT4F,
        PT_DIR      = TW_TYPE_DIR3F,
        //other stuff
        PT_BUTTON,
        PT_ENUM
    };
    ElemType tp;
    std::string name;
    std::string drawingHint;
    void *clientVar;
    std::function<void TW_CALL (void *)> buttonCallback;
    std::string enumTypes;
};

/**
 * @brief Encapsulates routines of state machine
 */
    class EntityState
    {
    public:
        EntityState(stateCallback eventCallback, Entity* _caller, float wait=0.0f, int returnIndex = 0)
        {
            assert(eventCallback != NULL && _caller != NULL && wait >= 0.0f);
            callback = eventCallback;
            caller = _caller;
            waitTime = wait; //if wait == 0.0, EventTimer is disabled
            retIndBackup = returnIndex; //retInd should acquire it's true value only after EventTimer!
            retInd = 0;
            obsolete = false;
            holdEx = false;
        }
        ~EntityState()
        {
        }
        void	handleEvents(std::list<EntityEvent*> &events)
        {
            if(waitTime > 0.0f) { waitTime -= egg::getInstance().g_Delta; if(waitTime == 0.0f) waitTime = -1.0f; }
            if(waitTime < 0.0f) { retInd = retIndBackup; events.push_back(new EventTimer()); }
            while(!events.empty())
            {
                callback(events.front(), caller);
                if(obsolete)
                { //happens when current state is popped
                    if(events.front()->eventCode == EventCode_Timer)
                    {
                        delete events.front();
                        events.pop_front();
                    }
                    return;
                }
                delete events.front();
                events.pop_front();
                if(holdEx) { holdEx = false; return; }
            }
        }
        
        //when event pushes another state, the previous one should
        //stop proccessing events and leave them for the new state
        void	holdExecution() { holdEx = true; }
        void	setObsolete() { obsolete = true; }
        
        int		getReturnIndex() const { return retInd; }
    private:
        Entity*						caller;
        stateCallback				callback;
        float						waitTime;
        int							retInd;
        int							retIndBackup;
        bool						obsolete;
        bool						holdEx;
    };

/**
 * @brief Base class for all objects in World
 */
class Entity : public Touchable, public Unique, public FamilyTree, public Serial
{
public:

			 Entity();
	virtual ~Entity();

    /** @brief Setup and load needed stuff
     * @return void
     */
	virtual void			initialize();

    /** @brief Perform entity-specific corrections of position/orientation etc
     * @return void
     */
	virtual void			adjustMoving();

    /** @brief Inform World, that this entity is to be deleted at the next update
     * @return void
     */
	void					destroy();

    /** @brief Inform entity about certain event
     * @param ee The event
     * @return void
     */
	void					sendEvent(EntityEvent* ee);

    /** @brief Hide entity and disable it's physics (if any)
     * @return void
     */
	void					switchToEditorModel();

    /** @brief Unhide entity and enable it's physics (if any)
     * @return void
     */
	void					switchToModel();

    /** @brief Deserializes class from JSON object
     * @param d Document object
     * @return void
     */
    virtual void Deserialize(rapidjson::Value& d);

    /** @brief Serializes class into JSON objects
     * @param d Document object, need for encoding and stuff
     * @return rapidjson::Value
     */
    virtual rapidjson::Value Serialize(rapidjson::Document& d);

    /** @brief Serialize class for copying in editor
     * @param d Document object, need for encoding and stuff
     * @return rapidjson::Value
     */
    virtual rapidjson::Value SerializeForCopying(rapidjson::Document& d);

    /** @brief Get entity's name
     * @return std::string
     */
    string getName();

    /** @brief Get physical body(@ref SolidBody) of entity
     * @return SolidBody*
     */
	const SolidBody*		getBody() const { return body; }

    /** @brief Get visual(@ref ModelSet) representation of entity
     * @return ModelSet*
     */
	const ModelSet*         getModelSet() const { return modelset; }

    /** @brief Check if entity can react to touching
     * @return bool
     */
	const inline bool       isTouchable() const { return touchable; }

    /** @brief Set entity's name
     * @param newName Sen
     * @return void
     */
    void					setName(std::string newName);

    /** @brief Create visual(@ref ModelSet) representation for this entity
     * @param modelConfigPath Path to model configuration file
     * @return void
     */
	void					setupModel(std::string modelConfigPath);

    /** @brief Let the physical body of this entity be in form of it's visual representation
     * @param mass Mass in kilos
     * @return void
     */
	void					setupCollision(float mass);

    /** @brief Let the physical body of this entity be in form sphere
     * @param mass Mass in kilos
     * @param radius Radius of sphere in meters
     * @return void
     */
	void					setupCollision(float mass, float &radius);

    /** @brief Let the physical body of this entity be in form of box
     * @param mass Mass in kilos
     * @param halfExtents Box half-sizes
     * @return void
     */
	void					setupCollision(float mass, glm::vec3 &halfExtents);

    /** @brief Attach this entity to another entity
     * @param p Entity to be attached to
     * @return void
     */
	void                    setParent(Entity* p);

    /** @brief Get the entity I am attached to
     * @return Entity*
     */
	Entity*                 getParent() const;

    /** @brief Set the position of both visual(@ref ModelSet) representation of entity, and it's physical body(@ref SolidBody)
     * @param pos Position in 3D space, units are meters
     * @return void
     */
	void                    setPosition(glm::vec3 pos);

    /** @brief Set the rotation of both visual(@ref ModelSet) representation of entity, and it's physical body(@ref SolidBody)
     * @param rot Rotation in 3D space, represented by quaternion
     * @return void
     */
	void                    setRotation(glm::quat rot);

    /** @brief Inform entity, that some pointer(@ref EntityPointer) is targeted at it
     * @param pen Pointer object, which target is this entity
     * @return void
     */
	void                    pointerAdded(EntityPointer* pen);

    /** @brief Inform entity, that some pointer(@ref EntityPointer) which was targeted at it, now points somewhere else
     * @param pen Pointer object, which betrayed this entity
     * @return void
     */
	void                    pointerLeft(EntityPointer* pen);

    virtual void            registerLua(LuaUserdata<Entity>& lua);

    template<class C>
    typename std::enable_if<std::is_base_of<FromLua, C>::value>::type addToLua(LuaUserdata<Entity>& l, string s, C c)
    {
        Lua *pLua = &egg::getInstance().g_lua;
        l.Set("get"+s, pLua->CreateFunction<LuaUserdata<C>()>([&, s]() {
            auto lud = pLua->CreateUserdata<C>(&(getProperty<C>(s)));
            if(lud.GetPointer() != nullptr) lud->registerLua(lud);
            return lud;
        }));
        l.Set("set"+s, pLua->CreateFunction<void(LuaUserdata<C>)>([&, s](LuaUserdata<C> arg) {
            setProperty<C>(s, *(arg.GetPointer()));
        }));
    }
    template<class C>
    typename std::enable_if<!std::is_base_of<FromLua, C>::value>::type addToLua(LuaUserdata<Entity>& l, string s, C c)
    {
        Lua *pLua = &egg::getInstance().g_lua;
        l.Set("get"+s, pLua->CreateFunction<C()>([&, s]() {
            return getProperty<C>(s);
        }));
        l.Set("set"+s, pLua->CreateFunction<void(C)>([&, s](C arg) {
            setProperty<C>(s, arg);
        }));
    }

    template<class C, class... T>
    void addToLua(LuaUserdata<Entity> &l, string s, C c, T... args)
    {
        addToLua(l, s, c);
        addToLua(l, args...);
    }

protected:
	//states:
	DECLARE_STATE(autowaitState);

    /** @brief (USE MACRO autowait INSTEAD) Internal method for suspending AI for a given period of time
     * @param time How much AI should be suspended
     * @param returnIndex Used for defining the point at which the AI should continue execution
     * @return void
     */
	void					_autowait(float time, int returnIndex);

    /** @brief Push new state in stack
     * @param callback State which handles the events
     * @param waitTime Period of time, after which a Timer Event is sent to the entity
     * @return void
     */
	void					pushState(stateCallback callback, float waitTime = 0.0f);

    /** @brief Replace current state in the stack with another
     * @param callback State which handles the events
     * @param waitTime Period of time, after which a Timer Event is sent to the entity
     * @return void
     */
	void					replaceState(stateCallback callback, float waitTime = 0.0f);

    /** @brief Removes current state from stack
     * @param ee Event to be passed to underlying state in stack
     * @return void
     */
	void					popState(EntityEvent* ee = nullptr);

    /** @brief Sync rotation of visual representation of entity and it's physical body
     * @return void
     */
	void					syncEntityRotation();

    /** @brief Process incoming events and update the entity
     * @return void
     */
        void			update();

    /** @brief Inform attached entities about position change
     * @return void
     */
	void                    updatePosition();

    /** @brief React to the change of position of the entity I am attached to
     * @return void
     */
        void                    parentMoved();

    /** @brief Recursively determine if specified entity is attached to this one
     * @param e Entity to check
     * @return void
     */
        bool                    childrenContain(Entity* e) const;

    /** @brief Register entity's pointers(@ref EntityPointer) as properties
     * @return void
     */
        void                    registerPointers();

        std::string             pointersString; //gui enum is made from this string

    /** @brief Get pointer(@ref EntityPointer) object at current Entity::pointerIndex
     * @return EntityPointer*
     */
        EntityPointer*          getTargetPointer();

    /** @brief Get description of entity at which current pointer at Entity::pointerIndex is targeted
     * @return std::string
     */
        std::string             getPointerDescr();

    /** @brief Perform editor-specific updates (i.e. props gui etc)
     * @return void
     */
        virtual void            editorUpdate();

    /** @brief Draw props gui for editor
     * @return void
     */
        virtual void            editorSelect();

    /** @brief Kill props gui for editor
     * @return void
     */
        virtual void            editorDesselect();

    /** @brief Render red arrow so above as below selected (this) entity
     * @return void
     */
	virtual void            renderSelectionIndicator();

	std::stack<EntityState*>	statesObsolete;
	std::stack<EntityState*>	states;
	std::list<EntityEvent*>		events;

	float						rotationSpeed;
	bool						editor;
	SolidBody*                  body;
	ModelSet*                   modelset;
    map<string, Property*>      properties;
    std::string					name;
    bool                        touchable;

    template<class T>
    T&                          getProperty(std::string)
    {
        return properties[name]->GetValue<T>();
    }

    template<class T>
    void                        setProperty(string s, T& val)
    {
        properties[s]->SetValue<T>(val);
    }

    /** @brief Register entity's properties
     * @return void
     */
    virtual void    addProperties();
    vector<vector<DrawableElement>> guiElements;

    /** @brief Display one prop gui element in a specific way
     * @param elem Element object to be displayed
     * @return void
     */
    void    drawSingleElement(DrawableElement &elem);

    /** @brief Display all prop gui elements(@ref DrawableElement)
     * @return void
     */
        void    drawGuiElements();

    /** @brief Register following gui elements
     * @param lle List of gui elements(@ref DrawableElement)
     * @return void
     */
    void    addDrawableElements(initializer_list<initializer_list<DrawableElement>> lle);

    // SOME BLACK MAGIC
    template<class C>
    void registerProperties(string s, C* c)
    {
        Property* pname = Property::create<C>(c);
        properties[s] = pname;
    }

    template<class C, class... T>
    void registerProperties(string s, C* c, T... Args)
    {
        registerProperties(s, c);
        registerProperties(Args...);
    }
    // END BLACK MAGIC


protected:
	glm::quat                   rotationQuat;
	glm::vec3                   rotationEuler;
	glm::quat                   rotationQuatO;
	glm::vec3                   rotationEulerO;
	glm::vec3                   position;
	std::vector<EntityPointer>  pointers;
	std::vector<EntityPointer*> pointAtMe;
	virtual void                fillPointers();
	int                         pointerIndex;
	int                         pointerIndexPrevious;
	bool                        shouldClearPointer;
	TwBar*                      entityBar;

	Entity*                     parent;
	std::vector<Entity*>        children;
	glm::mat4                   relativeTransform;
	glm::mat4                   oldMatrix;

	WorldGraphics*				wldGFX; //fill theese
	WorldPhysics*				wldPHY; //from class World
	World*						wld;

	friend class World;
	friend class Editor;
};

#endif
