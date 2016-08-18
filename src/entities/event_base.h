#ifndef EVENT_BASE_H_INCLUDED
#define EVENT_BASE_H_INCLUDED
#include <hash.h>

#define DECL_EVENTCODE(e) \
constexpr unsigned e ## _code = HASH(#e); \
unsigned e::_code = e ## _code

class Event {
public:
  Event() { code = _code; }
  virtual ~Event() = default;

  unsigned code;
  static unsigned _code;
};

#endif // EVENT_BASE_H_INCLUDED
