#include "touch_field.h"

IMPLEMENT_STATE(TouchField, WaitingEntry)
    switchEvent {
        case EventCode_Begin : {
            caller->active = true;
            return;
        }
        case EventCode_Deactivate : {
            caller->replaceState(Frozen);
            return;
        }
        case EventCode_Touch : {
            EventTouch *et = dynamic_cast<EventTouch*>(ee);
            EntityPointer &penEnter = caller->getProperty<EntityPointer>("Enter Target");
            if(penEnter)
            {
                penEnter->sendEvent(new EventTrigger(static_cast<Entity*>(et->toucher)));
            }
            if(caller->exitCheckTime > 0.0f) {
                caller->enLastIn = static_cast<Entity*>(et->toucher);
                caller->replaceState(WaitingExit, caller->exitCheckTime);
            }
            return;
        }
        default : return;
    }
END_STATE();

/*********************************************************/

IMPLEMENT_STATE(TouchField, WaitingExit)
    switchEvent {
        case EventCode_Deactivate : {
            caller->replaceState(Frozen);
            return;
        }
        case EventCode_Timer : {
            if(!caller->enLastIn || !caller->body->collidesWith(caller->enLastIn->getBody()) )
            {
                EntityPointer &penExit = caller->getProperty<EntityPointer>("Exit Target");
                if(penExit)
                {
                    penExit->sendEvent(new EventStart(caller->enLastIn));
                }
                caller->replaceState(WaitingEntry);
                return;
            }
            caller->replaceState(WaitingExit, caller->exitCheckTime);
            return;
        }
        default : return;
    }
END_STATE();

/*********************************************************/

IMPLEMENT_STATE(TouchField, Frozen)
    switchEvent {
        case EventCode_Begin : {
            caller->active = false;
            return;
        }
        case EventCode_Activate : {
            caller->replaceState(WaitingEntry);
            return;
        }
        default : return;
    }
END_STATE();

/*********************************************************/

IMPLEMENT_STATE(TouchField, main)
    switchEvent {
        case EventCode_Begin : {
            if(caller->active)
            {
                caller->replaceState(WaitingEntry);
            } else {
                caller->replaceState(Frozen);
            }
            return;
        }
        default : return;
    }
END_STATE();
