#include "entities/events.h"
#include "entity.h"

Entity::EntityState::EntityState()
{
    eventHandlers[0u] = ([](float, const EventPtr&) -> bool { return false; });
}

bool Entity::EntityState::CanHandle(unsigned eventcode) const
{
    return hasOtherwise ||
           HasSpecificHandler(eventcode);
}

bool Entity::EntityState::HasSpecificHandler(unsigned eventcode) const
{
    return eventHandlers.find(eventcode) != eventHandlers.end();
}

void Entity::EntityState::OnIdle(handler h)
{
    eventHandlers[0u] = h;
}

void Entity::EntityState::Otherwise(handler h)
{
    otherwiseHandler = h;
    hasOtherwise = true;
}

void Entity::EntityState::On(unsigned eventcode, handler h)
{
    eventHandlers[eventcode] = h;
}

void Entity::EntityState::Process(Entity* e, float d, std::vector<EventPtr> &events)
{
  if (events.empty()) {
    if (eventHandlers[0u](d, nullptr)) {
      e->_phase = 0;
      e->_waitTime = 0.0f;
    }
    return;
  }

  while (!events.empty()) {
    bool hasHandler = HasSpecificHandler(events.back()->code);

    if ((hasHandler || hasOtherwise) && e->stateChange == SC_NOCHANGE)
    {
      handler &handlerForEvent = (hasHandler ? eventHandlers[events.back()->code] : otherwiseHandler);
      if (!handlerForEvent(d, events.back())) {
          return;
      } else {
        e->_phase = 0;
        e->_waitTime = 0.0f;
        events.pop_back();
      }
    } else {
      return;
    }

  }
}
