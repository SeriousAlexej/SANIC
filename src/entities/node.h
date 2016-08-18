#ifndef NODE_H
#define NODE_H
#include "../entity.h"
#include <set>

class Node : public Entity
{
public:
    FROM_INCUBATOR(Node);
    
    Node();
    virtual ~Node();
    virtual void initialize() override;
    //virtual void editorUpdate() override;
    void Deserialize(rapidjson::Value& d) override;
    
private:
    //states
    DECLARE_STATE(main);
    //~states
    virtual void            editorUpdate() override;
    void                    drawConnections();
    bool                    drawn = false;
    int                     connections;
    
    friend class Node;
};

#endif // NODE_H
