#ifndef EVENTS_H_INCLUDED
#define EVENTS_H_INCLUDED
#include "event_base.h"
#include "entitypointer.h"

class EBegin : public Event {
public:
  EBegin() { code = _code; }
  virtual ~EBegin() = default;

  static unsigned _code;
};

class EActivate : public Event {
public:
  EActivate() { code = _code; }
  virtual ~EActivate() = default;

  static unsigned _code;
};

class EDeactivate : public Event {
public:
  EDeactivate() { code = _code; }
  virtual ~EDeactivate() = default;

  static unsigned _code;
};

class ETimer : public Event {
public:
  ETimer() { code = _code; }
  virtual ~ETimer() = default;

  static unsigned _code;
};

class ETouch : public Event {
public:
    ETouch() { code = _code; }
    virtual ~ETouch() = default;

    EntityPointer who;

    static unsigned _code;
};

#endif // EVENTS_H_INCLUDED
