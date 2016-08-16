#ifndef EDITOR_H
#define EDITOR_H
#include "eggineinstance.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <unordered_map>
#include <functional>
#include <luacppinterface.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>
#include "global.h"

class World;
class InputHandler;
class Entity;
struct RayCastInfo;

class Editor : public EggineInstance, public std::enable_shared_from_this<Editor>
{
public:
    typedef std::shared_ptr<Editor> Ptr;

    static Ptr Create();
    virtual ~Editor();

    int         run();
    void        setup();
    void        NewWorld();
    void        Load();
    void        SaveAs();
    void        Save();

private:
    class BasicAction {
    public:
        std::string name;
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
        LuaAction(std::string n, LuaFunction<void()> f) : func(f){
            name = n;
        }
    };

    enum EditorMode { PickEntity, Fly, Moving, Pulling, Idle, LuaMenu, Menu };

    Editor();

    void        setupGUI();

    void        copyEntity();
    void        pasteEntity();

    void        update();

    void        drawGrid() const;

    void        loadAddons();
    void        registerLua();

    void        resizeGUIComponents(unsigned width, unsigned height);

    void        eventsAlways(sf::Event &event);
    void        eventsLuaMenu(sf::Event &event);
    void        eventsIdle(sf::Event &event);
    void        eventsMenu(sf::Event &event);
    void        eventsFly(sf::Event &event);
    void        eventsMoving(sf::Event &event);
    void        eventsPulling(sf::Event &event);
    void        eventsPick(sf::Event &event);

    void        fillActionsMenu();
    void        changeMode(EditorMode newMode);

    void        selectEntity(Entity* en);

    RayCastInfo castRayScreen(bool fromCenter = false);

    glm::vec3                       vCameraSpeed;
    std::string                     currentWorld;
    float                           doubleClickTime;
    float                           moveModeTime;
    float                           moveModePeriod;
    World*                          p_world;
    InputHandler*                   p_input;
    sf::RenderWindow*               window;
    sfg::Desktop                    desktop;
    Entity*                         selectedEntity;
    std::string                     pointerProp;
    std::string                     copyEntitySerialized;
    float                           editorFlySpeed;
    glm::vec2                       mposOffsetMoving;
    EditorMode                      edMode;
    std::shared_ptr<sfg::Table>     tableEntityProperties;
    std::shared_ptr<sfg::Window>    leftWindow;
    std::shared_ptr<sfg::Window>    topWindow;
    std::shared_ptr<sfg::Window>    popup;
    std::shared_ptr<sfg::Label>     modeStatus;

    std::vector<std::shared_ptr<sfg::Window>>   custom;
    std::unordered_map<char, LuaAction>         actions;
    std::unordered_map<char, LuaAction>         ctrl_actions;
};

#endif // EDITOR_H
