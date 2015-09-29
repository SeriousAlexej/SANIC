#ifndef EDITOR_H
#define EDITOR_H
#include "eggineinstance.h"
#include <glm/glm.hpp>
#include <map>
#include <functional>
#include <luacppinterface.h>
#include <SFML/Window.hpp>
#include "global.h"

using std::map;
using std::string;

class World;
class InputHandler;
class Entity;
struct RayCastInfo;
namespace sfg
{
    class Window;
    class Label;
}

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
    void loadAddons();
    void registerLua();

    class BasicAction {
    public:
        string name;
        virtual void run() = 0;
    };

    class LuaAction : public BasicAction {
    private:
        LuaFunction<void()> func;
    public:
        virtual void run() override {
            std::cout << "Invoke!" << std::endl;
            func.Invoke();
        }
        LuaAction() : func(egg::getInstance().g_lua.CreateFunction<void()>([&] {return;})) {}
        LuaAction(string n, LuaFunction<void()> f) : func(f){
            name = n;
        }
    };

    map<char, LuaAction> actions;
    map<char, LuaAction> ctrl_actions;

	void resizeGUIComponents(unsigned width, unsigned height);

    void eventsAlways(sf::Event &event, sf::Window &window);
    void eventsMenu(sf::Event &event);
    void eventsIdle(sf::Event &event);

    World* p_world;
	InputHandler* p_input;
    RayCastInfo castRayScreen(bool fromCenter = false);
    void updateEntity(Entity* pen);

    enum EditorMode { Fly, Moving, Pulling, Idle, InMenu };
    void changeMode(EditorMode newMode);
    std::shared_ptr<sfg::Label> modeStatus;

	Entity*					selectedEntity;
	float					editorFlySpeed;
	glm::vec2               mposOffsetMoving;
	EditorMode 				edMode;
	std::shared_ptr<sfg::Window> leftWindow;
	std::shared_ptr<sfg::Window> topWindow;
};

#endif // EDITOR_H
