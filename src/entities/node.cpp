#include "node.h"
#include <sstream>

ADD_TO_INCUBATOR(Node);

Node::Node()
{
    setClass("Node");
    connections = 0;
}

void Node::main(EntityEvent* ee, Entity* caller)
{
}


Node::~Node()
{
}

void Node::initialize()
{
    setupModel("./shaders/fullbright.shader",
            "./models/editor/node.obj",
            "./models/editor/node.png",
            "", "");
    
    switchToEditorModel();
    
    if(egg::getInstance().g_Editor)
    {
	    setupCollision(0.0f, glm::vec3(0.25f, 0.25f, 0.25f));
    }
    
	pushState(main);
	addProperties();
}

void Node::drawConnections()
{
    if(drawn) return;
    drawn = true;
    for(auto& ptr : pointers) if(ptr) {
        glm::vec3 pos = getPosition();
        glm::vec3 pos2 = ptr->getPosition();
        glm::vec3 delta = pos2-pos;
        glPushMatrix();
        glLineWidth(2.0f);
        glTranslatef(pos.x, pos.y, pos.z);
        glBegin(GL_LINES);
            glColor3f(1,0,0);
            glVertex3d(0, 0, 0);
            glVertex3d(delta.x, delta.y, delta.z);
        glEnd();
        glLineWidth(1.0f);
        glPopMatrix();
        
        Node* n = (Node*)*ptr;
        n->drawConnections();
    }
    drawn = false;
}

void Node::editorUpdate()
{
    drawConnections();
    Entity::editorUpdate();

}

void Node::Deserialize(rapidjson::Value& d)
{
    Entity::Deserialize(d);
    connections = pointers.size()-1;
}

