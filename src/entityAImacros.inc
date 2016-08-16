#define ONIDLE(state)\
states[state].OnIdle([this](float d, const EventPtr& event) -> bool {\
switch(_phase){case 0:{

#define ON(state, event)\
states[state].On(event::_code, [this](float d, const EventPtr& event) -> bool {\
switch(_phase){case 0:{

#define PASS() \
popState(); _phase = 0; _waitTime = 0.0f; return false;

#define ONOTHER(state)\
states[state].Otherwise([this](float d, const EventPtr& event) -> bool {\
switch(_phase){case 0:{

#define WAIT(x)\
_waitTime = x;_phase=__LINE__;return false;} case __LINE__: {_waitTime-=d; if(_waitTime <= 0.0f) _phase++; else return false;} case __LINE__+1:{

#define ENDON \
} default: return true;} return true; } );

#define PUSH0(s)\
return pushState(&states[s]);

#define PUSHE(s, e)\
pushState(&states[s]); sendEvent(MAKEEVENT(e)); return true;

#define POP0()\
return popState();

#define POPE(e)\
popState(); sendEvent(MAKEEVENT(e)); return true;

#define SWAP0(s)\
return swapState(&states[s]);

#define SWAPE(s, e)\
swapState(&states[s]); sendEvent(MAKEEVENT(e)); return true;

#define GET_MACRO01(_0, _1, NAME, ...) NAME
#define GET_MACRO12(_1, _2, NAME, ...) NAME
#define POP(...) GET_MACRO01(_0, ##__VA_ARGS__, POPE, POP0)(__VA_ARGS__)
#define PUSH(...) GET_MACRO12(__VA_ARGS__, PUSHE, PUSH0)(__VA_ARGS__)
#define SWAP(...) GET_MACRO12(__VA_ARGS__, SWAPE, SWAP0)(__VA_ARGS__)

#define MAKEEVENT(e)\
std::make_unique< ::e>()

#define SENDEVENT(e)\
sendEvent(std::move(e))
