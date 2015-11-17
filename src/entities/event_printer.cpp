#include "event_printer.h"

ADD_TO_INCUBATOR(EventPrinter);

EventPrinter::EventPrinter()
{
}

EventPrinter::~EventPrinter()
{
}

void EventPrinter::initialize()
{
    pushState(main);
    setupModel("./models/editor/ep.mconf");
    setupCollision(0.0f);
    addProperties();
    setName("EventPrinter");
}

IMPLEMENT_STATE(EventPrinter, main)
    switchEvent {
        case EventCode_Trigger : {
            EventTrigger *et = dynamic_cast<EventTrigger*>(ee);
            printf("Got event trigger from %s\n", et->penCaused->getName().c_str());
            return;
        }
        case EventCode_Start : {
            EventStart *es = dynamic_cast<EventStart*>(ee);
            printf("Got event start from %s\n", es->penCaused->getName().c_str());
            return;
        }
        default : return;
    }
END_STATE();
