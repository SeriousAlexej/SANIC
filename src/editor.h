#ifndef EDITOR_H
#define EDITOR_H
#include "eggineinstance.h"
#include <glm/glm.hpp>
#include <map>
#include <functional>
#include <luacppinterface.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>
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
    class Desktop;
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

    void eventsAlways(sf::Event &event);
    void eventsLuaMenu(sf::Event &event);
    void eventsIdle(sf::Event &event);
    void eventsMenu(sf::Event &event);
    void eventsFly(sf::Event &event);
    void eventsMoving(sf::Event &event);
    void eventsPulling(sf::Event &event);

    void fillActionsMenu();

    World* p_world;
	InputHandler* p_input;
    sf::RenderWindow* window;
    sfg::Desktop desktop;

    RayCastInfo castRayScreen(bool fromCenter = false);
    void updateEntity(Entity* pen);

    enum EditorMode { Fly, Moving, Pulling, Idle, LuaMenu, Menu };
    void changeMode(EditorMode newMode);
    std::shared_ptr<sfg::Label> modeStatus;

    glm::vec3                   vCameraSpeed;
    float                       fLastClick;

    std::shared_ptr<sfg::Window> popup;
    std::vector<std::shared_ptr<sfg::Window>> custom;

	Entity*					selectedEntity;
	float					editorFlySpeed;
	glm::vec2               mposOffsetMoving;
	EditorMode 				edMode;
	std::shared_ptr<sfg::Window> leftWindow;
	std::shared_ptr<sfg::Window> topWindow;
};

#endif // EDITOR_H
