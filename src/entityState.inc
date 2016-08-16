#ifndef ENTITYSTATE_INC_INCLUDED
#define ENTITYSTATE_INC_INCLUDED

class EntityState {
public:
    typedef std::function<bool(float, const EventPtr&)> handler;

    EntityState();
    ~EntityState() = default;

    bool CanHandle(unsigned eventcode) const;
    void OnIdle(handler h);
    void Otherwise(handler h);
    void On(unsigned eventcode, handler h);
    void Process(Entity* e, float d, std::vector<EventPtr> &events);

private:
    bool HasSpecificHandler(unsigned eventcode) const;

    bool                                  hasOtherwise = false;
    handler                               otherwiseHandler;
    std::unordered_map<unsigned, handler> eventHandlers;
};

#endif // ENTITYSTATE_INC_INCLUDED
