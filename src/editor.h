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

class Editor : public EggineInstance, public std::enable_shared_from_this<Editor>
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

private:
    Editor();
	void update();

	void resizeGUIComponents(unsigned width, unsigned height);

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
};

#endif // EDITOR_H
