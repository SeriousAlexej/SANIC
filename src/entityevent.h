#ifndef _ENTITY_EVENT_H_
#define _ENTITY_EVENT_H_
#include <luacppinterface.h>
#include <memory>
#include "entitypointer.h"

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
    EntityPointer penDamager;
    int amount;
    EventDamage(Entity* pen, int a) : amount(a)
    {
        eventCode = EventCode_Damage;
        penDamager = pen;
    }
    EventDamage(EntityPointer &pen, int a) : amount(a)
    {
        eventCode = EventCode_Damage;
        penDamager = pen;
    }
};


/*********************************************************************/

const int EventCode_Trigger = 6;
class EventTrigger : public EntityEvent
{
public:
    EntityPointer penCaused;
    EventTrigger(Entity* pen)
    {
        eventCode = EventCode_Trigger;
        penCaused = pen;
    }
    EventTrigger(EntityPointer &pen)
    {
        eventCode = EventCode_Trigger;
        penCaused = pen;
    }
};


/*********************************************************************/

const int EventCode_Start = 7;
class EventStart : public EntityEvent
{
public:
    EntityPointer penCaused;
    EventStart(Entity* pen)
    {
        eventCode = EventCode_Start;
        penCaused = pen;
    }
    EventStart(EntityPointer &pen)
    {
        eventCode = EventCode_Start;
        penCaused = pen;
    }
};

/*********************************************************************/

const int EventCode_Activate = 8;
class EventActivate : public EntityEvent
{
public:
    EventActivate()
    {
        eventCode = EventCode_Activate;
    }
};

/*********************************************************************/

const int EventCode_Deactivate = 9;
class EventDeactivate : public EntityEvent
{
public:
    EventDeactivate()
    {
        eventCode = EventCode_Deactivate;
    }
};

/*********************************************************************/

const int EventCode_Stop = 10;
class EventStop : public EntityEvent
{
public:
    EventStop()
    {
        eventCode = EventCode_Stop;
    }
};

/*********************************************************************/

#endif
