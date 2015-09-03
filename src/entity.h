#ifndef _ENTITY_H_
#define _ENTITY_H_
#include <list>
#include <stack>
#include <string>
#include <map>
#include <initializer_list>
#include <AntTweakBar.h>
#include "basic.h"
#include "solidbody.h"
#include "modelinstance.h"
#include "entityevent.h"
#include "world_graphics.h"
#include "world_physics.h"
#include "world.h"
#include "properties.h"
#include "entitypointer.h"
#include "entities/incubator.h"
#include <luacppinterface.h>
#include <type_traits>
#include "global.h"

class Entity;
class EntityPointer;
class World;

typedef void (*stateCallback)(EntityEvent*,Entity*);
typedef void STATE;
#define DECLARE_STATE(x) \
    static void x(EntityEvent* ee, Entity* caller)

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
		if(waitTime < 0.0f){ retInd = retIndBackup; events.push_back(new EventTimer()); }
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

class Entity : public Touchable, public Unique, public FamilyTree, public Serial, protected FromIncubator
{
public:

			 Entity();
	virtual ~Entity();
	virtual void			initialize();
	virtual void			adjustMoving();

	void					destroy();

	void					sendEvent(EntityEvent* ee);

	void					switchToEditorModel();
	void					switchToModel();

    virtual void Deserialize(rapidjson::Value& d);
    virtual rapidjson::Value Serialize(rapidjson::Document& d);

    string getName();
	const SolidBody*		getBody() const { return body; }//for moving purposes
	const ModelInstance*	getModelInstance() const { return model; }//for anim playing

    void					setName(std::string newName);

	void					setupModel(std::string shaderPath,
									   std::string modelPath, std::string diffTexture,
									   std::string normTexture, std::string heightTexture);
	void					setupCollision(float mass); //from model
	void					setupCollision(float mass, float radius); //sphere
	void					setupCollision(float mass, glm::vec3 halfExtents); //box

	void                    setParent(Entity* p);
	Entity*                 getParent() const;

	void                    setPosition(glm::vec3 pos);
	void                    setRotation(glm::quat rot);

	void                    pointerAdded(EntityPointer* pen);
	void                    pointerLeft(EntityPointer* pen);

    virtual void            registerLua(LuaUserdata<Entity>& lua);

    template<class C>
    typename std::enable_if<std::is_base_of<FromLua, C>::value>::type addToLua(LuaUserdata<Entity>& l, string s, C c)
    {
        Lua *pLua = &egg::getInstance().g_lua;
        l.Set("get"+s, genUserdataGetter(*pLua, &getProperty<C>(s)));
        l.Set("set"+s, pLua->CreateFunction<void(LuaUserdata<C>)>([&, s](C arg) {
            setProperty<C>(s, arg);
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
	DECLARE_STATE(dummy);
	DECLARE_STATE(autowaitState);

	void					autowait(float time, int returnIndex);
	void					pushState(stateCallback callback, float waitTime = 0.0f);
	void					replaceState(stateCallback callback, float waitTime = 0.0f);
	void					popState(EntityEvent* ee = nullptr);
	void					syncEntityRotation();
	void					update();
	void                    updatePosition(); //inform children about position change
	void                    parentMoved();
	bool                    childrenContain(Entity* e) const;

    void                    registerPointers();
    std::string             pointersString; //gui enum is made from this string
    EntityPointer*          getTargetPointer();
    std::string             getPointerDescr();
	virtual void            editorUpdate();
	virtual void            editorSelect();
	virtual void            editorDesselect();
	virtual void            renderSelectionIndicator();

	std::stack<EntityState*>	statesObsolete;
	std::stack<EntityState*>	states;
	std::list<EntityEvent*>		events;

	float						rotationSpeed;
	bool						editor;
	SolidBody*					body;
	ModelInstance*				model;
    map<string, Property*>      properties;
    std::string					name;

    template<class T>
    T&                          getProperty(std::string);

    template<class T>
    void                        setProperty(string s, T& val);

    virtual void    addProperties();
    vector<vector<DrawableElement>> guiElements;
    void    drawSingleElement(DrawableElement &elem);
    void    drawGuiElements();

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
