#ifndef EDITOR_H
#define EDITOR_H
#include "eggineinstance.h"
#include <glm/glm.hpp>

class World;
class InputHandler;
class Entity;
struct RayCastInfo;
namespace sfg
{
    class Window;
};

class Editor : public EggineInstance
{
public:
    typedef std::shared_ptr<Editor> Ptr;
    static Ptr Create();
    virtual ~Editor();

    int run();
    void setup();
    void NewWorld();
    void Load();
    void SaveAs();
    void Save();

private:
    Editor();
	void update();

	void resizeGUIComponents(unsigned width, unsigned height);
	void copyEntity();
	void pasteEntity();
	void spawnEntity(std::string classname);

    World* p_world;
	InputHandler* p_input;
    RayCastInfo castRayScreen(bool fromCenter = false);
    void updateEntity(Entity* pen);

	enum EditorMode { Fly, Moving, Pulling, Idle };

	Entity*					selectedEntity;
	float					editorFlySpeed;
	glm::vec2               mposOffsetMoving;
	EditorMode 				edMode;
	std::shared_ptr<sfg::Window> leftWindow;
	std::shared_ptr<sfg::Window> topWindow;
	std::string             copyEntitySerialized="";
	std::string             currentWorld="";

	friend class IncuButton;
};

#endif // EDITOR_H
