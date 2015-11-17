#include <string>
#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>
#include <AntTweakBar.h>
#include "dialogs/tinyfiledialogs.h"
#include "world.h"
#include "input_handler.h"
#include "editor.h"
#include "entity.h"
#include "global.h"
#include "editorGUI.h"
#include <dirtools.h>
#include <istream>

class IncuButton : public sfg::Button
{
public:
    typedef std::shared_ptr<IncuButton> Ptr;
    static Ptr Create(const sf::String &lbl, World *w)
    {
        std::shared_ptr<IncuButton> ib(new IncuButton(w));
        ib->SetLabel(lbl);
        return ib;
    }
    void Spawn()
    {
        assert(wld != nullptr);
        wld->createEntity(GetLabel().toAnsiString());
    }
private:
    IncuButton(World *w) : wld(w) {}
    World *wld;
};


void TW_CALL CopyStdStringToClient(std::string &destinationClientString, const std::string &sourceLibraryString)
{
    destinationClientString = sourceLibraryString;
}

static std::string relativePath(std::string absPath)
{
    std::replace(absPath.begin(), absPath.end(), '\\', '/');
    if (absPath.find(egg::getInstance().g_WorkingDir) == 0) {
        absPath = "." + absPath.substr(egg::getInstance().g_WorkingDir.length());
    }
    if (absPath[0] != '.') {
        absPath = "." + absPath;
    }
    return absPath;
}

void Editor::NewWorld()
{
    if (selectedEntity != nullptr) {
        selectedEntity->editorDesselect();
        selectedEntity = nullptr;
    }
    p_world->Clear();
}

void Editor::Load()
{
    std::string path;
    const char *result = tinyfd_openFileDialog("Load world", "./", 0, NULL, 0);
    if (result) {
        path = relativePath(result);
        p_world->Love(path);
    }
}

void Editor::SaveAs()
{
    std::string path;
    const char *result = tinyfd_saveFileDialog("Save world as", "./", 0, NULL);
    if (result) {
        path = relativePath(result);
        p_world->Save(path);
    }
}

void Editor::resizeGUIComponents(unsigned width, unsigned height)
{
    TwWindowSize(width, height);

    topWindow->SetAllocation(sf::FloatRect(0.0f, 0.0f,
                                           static_cast<float>(width), static_cast<float>(topWndHeight)));

    egg::getInstance().g_DrawOrigin.x = 0u;
    egg::getInstance().g_DrawOrigin.y = 0u;

    egg::getInstance().g_Resolution.x = width;
    egg::getInstance().g_Resolution.y = height - topWndHeight;

    if (selectedEntity != nullptr) {
        std::string barSize = "size='" + std::to_string(barWidth) + " " + std::to_string(egg::getInstance().g_Resolution.y) + "' ";
        TwDefine((" EntityBar " + barSize).c_str());
    }
}

void Editor::setup()
{

}

void Editor::eventsAlways(sf::Event &event)
{
    switch (event.type) {
    case sf::Event::LostFocus: {
        p_input->setFocus(false);
        break;
    }
    case sf::Event::GainedFocus: {
        p_input->setFocus(true);
        break;
    }
    case sf::Event::Closed: {
        window->close();
        break;
    }
    case sf::Event::MouseWheelMoved: {
        p_input->registerWheelDelta(event.mouseWheel.delta);
        break;
    }
    case sf::Event::Resized: {
        unsigned width(event.size.width), height(event.size.height);
        bool fixSize = false;
        if (width < wndMinWidth) {
            fixSize = true;
            width = wndMinWidth;
        }
        if (height < wndMinHeight) {
            fixSize = true;
            height = wndMinHeight;
        }
        if (fixSize) {
            window->setSize(sf::Vector2u(width, height));
        }
        resizeGUIComponents(width, height);
        break;
    }
    default:
        desktop.HandleEvent(event);
        break;
    }
}

void Editor::eventsLuaMenu(sf::Event &event)
{
    switch (event.type) {
    case sf::Event::TextEntered:
        if (event.text.unicode < 128) {
            char key = static_cast<char>(event.text.unicode);
            if (actions.count(key) == 0) {
                changeMode(Idle);
                popup->Show(false);
            } else {
                actions[key].run();
                std::cout << "NONCTRL" << std::endl;
                changeMode(Idle);
                popup->Show(false);
            }
        }
        return;
    }
}

void Editor::eventsMenu(sf::Event &event)
{

}

void Editor::eventsIdle(sf::Event &event)
{
    switch (event.type) {
    case sf::Event::KeyPressed: {
        if (event.key.code == sf::Keyboard::Escape) {
            p_input->switchLockMouse(true);
            changeMode(Fly);
        }
        if (p_input->cursorIsInsideWindow()) {
            if (event.key.code == sf::Keyboard::Q) {
                if (selectedEntity != NULL) {
                    selectedEntity->editorDesselect();
                    selectedEntity = NULL;
                    return;
                }
            }

            if (selectedEntity != NULL && event.key.code == sf::Keyboard::R) {
                RayCastInfo ri = castRayScreen();
                EntityPointer *ep = selectedEntity->getTargetPointer();
                if (ep != nullptr) {
                    if (ep->Name() == "Parent") {
                        selectedEntity->setParent(ri.enHit);
                    } else {
                        (*ep) = ri.enHit;
                    }
                    selectedEntity->pointerIndexPrevious = -1; //update target info string
                }
            }
            if (event.key.code == sf::Keyboard::Delete) {
                if (selectedEntity != NULL) {
                    selectedEntity->editorDesselect();
                    p_world->removeEntity(selectedEntity);
                    selectedEntity = NULL;
                    return;
                }
            }
        }
        break;
    }
    case sf::Event::MouseButtonPressed: {
        if (event.mouseButton.button == sf::Mouse::Right) {
            if (selectedEntity != NULL && p_input->keyPressed(sf::Keyboard::LControl)) {
                changeMode(Pulling);
                sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow);
                mposOffsetMoving = glm::vec2(mpos.x, mpos.y);
            }
        } else if (event.mouseButton.button == sf::Mouse::Left) {
            static float moveModeTime = fLastClick;
            auto &graphics = *p_world->pGraphics;

            float tmNow = egg::getInstance().g_Clock.getElapsedTime().asSeconds();
            if (selectedEntity != NULL && p_input->keyPressed(sf::Keyboard::LControl)) {
                if (tmNow - fLastClick <= 0.5f) {
                    RayCastInfo ri = castRayScreen();
                    if (ri.enHit != NULL) {
                        selectedEntity->position = ri.posHit;
                    } else {
                        selectedEntity->position = ri.posOrigin + ri.direction * 5.0f;
                    }
                    return;
                }
                fLastClick = tmNow;

                changeMode(Moving);
                moveModeTime = tmNow;
                sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow);
                mpos.y = p_input->windowSize.y - mpos.y;
                glm::mat4 camMat = graphics.getCamera()->getProjectionMatrix() * graphics.getCamera()->getViewMatrix();
                glm::vec4 entPos = glm::vec4(selectedEntity->position.x, selectedEntity->position.y, selectedEntity->position.z, 1.0f);
                entPos =  camMat * entPos;
                entPos /= entPos.w;
                mposOffsetMoving = glm::vec2(entPos.x - (float(mpos.x) / float(p_input->windowSize.x)) * 2.0f + 1.0f,
                                             entPos.y - (float(mpos.y) / float(p_input->windowSize.y)) * 2.0f + 1.0f);
            } else {
                RayCastInfo rci = castRayScreen();
                if (rci.enHit != NULL) {
                    if (selectedEntity != nullptr) {
                        if (selectedEntity == rci.enHit) return;
                        selectedEntity->editorDesselect();
                    }
                    selectedEntity = rci.enHit;
                    selectedEntity->editorSelect();
                } else {
                    if (selectedEntity != nullptr) {
                        selectedEntity->editorDesselect();
                        selectedEntity = nullptr;
                    }
                }
            }
        }
        break;
    }
    case sf::Event::TextEntered: {
        char key;
        if (event.text.unicode >= 128) return;
        else key = static_cast<char>(event.text.unicode);
        if (key == 'a') {
            auto pos = sf::Mouse::getPosition(*window);
            popup->SetPosition({(float)pos.x, (float)pos.y});
            popup->SetTitle("Actions");
            fillActionsMenu();
            popup->Show(true);
            changeMode(LuaMenu);
            return;
        } else if (key == 'e') {
            auto pos = sf::Mouse::getPosition(*window);
            leftWindow->SetPosition({(float)pos.x, (float)pos.y});
            leftWindow->Show(!leftWindow->IsGloballyVisible());
            return;
        }
        break;
    }
    }
}

void Editor::eventsMoving(sf::Event &event)
{
    switch (event.type) {
    case sf::Event::MouseButtonReleased:
        if (event.mouseButton.button == sf::Mouse::Left) {
            changeMode(Idle);
        }
        break;
    }
}

void Editor::eventsPulling(sf::Event &event)
{
    switch (event.type) {
    case sf::Event::MouseButtonReleased:
        if (event.mouseButton.button == sf::Mouse::Right) {
            changeMode(Idle);
        }
        break;
    }
}

void Editor::eventsFly(sf::Event &event)
{
    switch (event.type) {
    case sf::Event::KeyPressed: {
        switch (event.key.code) {
        case sf::Keyboard::Up:
        case sf::Keyboard::W: {
            vCameraSpeed.z += 1;
            break;
        }
        case sf::Keyboard::Down:
        case sf::Keyboard::S: {
            vCameraSpeed.z += -1;
            break;
        }
        case sf::Keyboard::Right:
        case sf::Keyboard::D: {
            vCameraSpeed.x += 1;
            break;
        }
        case sf::Keyboard::Left:
        case sf::Keyboard::A: {
            vCameraSpeed.x += -1;
            break;
        }
        case sf::Keyboard::Space: {
            vCameraSpeed.y += 1;
            break;
        }
        case sf::Keyboard::C: {
            vCameraSpeed.y += -1;
            break;
        }
        case sf::Keyboard::Escape: {
            changeMode(Idle);
            p_input->switchLockMouse(false);
            vCameraSpeed = { 0, 0, 0 };
            break;
        }
        }   // event.key.code
        break;
    }   // case sf::Event::KeyPressed
    case sf::Event::KeyReleased: {
        switch (event.key.code) {
        case sf::Keyboard::Up:
        case sf::Keyboard::W: {
            if (vCameraSpeed.z != 0) vCameraSpeed.z -= 1;
            break;
        }
        case sf::Keyboard::Down:
        case sf::Keyboard::S: {
            if (vCameraSpeed.z != 0) vCameraSpeed.z -= -1;
            break;
        }
        case sf::Keyboard::Right:
        case sf::Keyboard::D: {
            if (vCameraSpeed.x != 0) vCameraSpeed.x -= 1;
            break;
        }
        case sf::Keyboard::Left:
        case sf::Keyboard::A: {
            if (vCameraSpeed.x != 0) vCameraSpeed.x -= -1;
            break;
        }
        case sf::Keyboard::Space: {
            if (vCameraSpeed.y != 0) vCameraSpeed.y -= 1;
            break;
        }
        case sf::Keyboard::C: {
            if (vCameraSpeed.y != 0) vCameraSpeed.y -= -1;
            break;
        }
        }   // event.key.code
        break;
    }       // case sf::Event::KeyReleased
    case sf::Event::MouseWheelScrolled: {
        double delta = event.mouseWheelScroll.delta;
        if (delta > 0)       editorFlySpeed = min(100.0f, editorFlySpeed * 2.0f);
        else if (delta < 0)  editorFlySpeed = max(0.25f, editorFlySpeed * 0.5f);
        break;
    }       // case sf::Event::MouseWheelScrolled
    }           // event.type
}

void Editor::fillActionsMenu()
{
    auto box = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
    std::map<string, char> names;
    for (auto kv : actions) names[kv.second.name] = kv.first;
    for (auto kv : names) {
        auto button = sfg::Button::Create(kv.first + " (" + kv.second + ")");
        LuaAction *action = &actions[kv.second];
        button->GetSignal(sfg::Button::OnLeftClick).Connect([action, this]() {
            action->run();
            popup->Show(false);
        });
        box->Pack(button);
    }
    popup->RemoveAll();
    popup->Add(box);
}

int Editor::run()
{
    printf(egg::getInstance().logo.c_str());

    sf::ContextSettings cs;
    cs.antialiasingLevel = 4;
    cs.depthBits = 24;
    cs.majorVersion = 3;
    cs.minorVersion = 0;

    window = new sf::RenderWindow(sf::VideoMode(egg::getInstance().g_Resolution.x, egg::getInstance().g_Resolution.y), "Eggine Editor", sf::Style::Default, cs);
    window->setVerticalSyncEnabled(true);
    window->setFramerateLimit(60);
    window->setActive();
    window->setKeyRepeatEnabled(false);

    glewExperimental = true;
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    TwInit(TW_OPENGL, NULL);
    TwCopyStdStringToClientFunc(CopyStdStringToClient);

    p_input = new InputHandler(window);
    p_world = new World();

    sfg::SFGUI sfgui;
    window->setActive();
    desktop.SetProperty("*", "FontSize", 14.0f);

    leftWindow = sfg::Window::Create();
    leftWindow->SetTitle("Create entity");
    topWindow = sfg::Window::Create(sfg::Window::Style::BACKGROUND);
    //fill windows here
    {
        auto leftBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 2.0f);
        std::vector<std::string> classes = Incubator::getRegisteredClasses();
        for (std::string &cl : classes) {
            auto btn = IncuButton::Create(cl, p_world);
            btn->GetSignal(sfg::Widget::OnLeftClick).Connect(std::bind(&IncuButton::Spawn, btn));
            leftBox->Pack(btn, true, true);
        }
        leftWindow->Add(leftBox);
        leftWindow->Show(false);

        auto topBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 2.0f);
        auto btnNew = sfg::Button::Create("New World");
        auto btnLoad = sfg::Button::Create("Load World");
        auto btnSaveAs = sfg::Button::Create("Save As");
        auto btnSave = sfg::Button::Create("Save");
        modeStatus = sfg::Label::Create("Idle");
        popup = sfg::Window::Create(sfg::Window::TOPLEVEL);
        popup->Show(false);

        btnNew->SetRequisition(sf::Vector2f(100.0f, 0.0f));
        btnLoad->SetRequisition(sf::Vector2f(100.0f, 0.0f));
        btnSaveAs->SetRequisition(sf::Vector2f(100.0f, 0.0f));
        btnSave->SetRequisition(sf::Vector2f(100.0f, 0.0f));
        btnNew->GetSignal(sfg::Widget::OnLeftClick).Connect(std::bind(&Editor::NewWorld, this));
        btnLoad->GetSignal(sfg::Widget::OnLeftClick).Connect(std::bind(&Editor::Load, this));
        btnSaveAs->GetSignal(sfg::Widget::OnLeftClick).Connect(std::bind(&Editor::SaveAs, this));
        //TODO: bind Save button
        topBox->Pack(btnNew, false, true);
        topBox->Pack(btnLoad, false, true);
        topBox->Pack(btnSaveAs, false, true);
        topBox->Pack(btnSave, false, true);
        topBox->Pack(modeStatus, true, true);

        topWindow->Add(topBox);

    }
    //~fill windows here
    desktop.Add(leftWindow);
    desktop.Add(topWindow);
    desktop.Add(popup);
    resizeGUIComponents(window->getSize().x, window->getSize().y);
    desktop.Update(0.f);

    loadAddons();

    if (!startupWorld.empty()) {
        p_world->Love(startupWorld);
    }


    egg::getInstance().g_Clock.restart();
    fLastClick = egg::getInstance().g_Clock.getElapsedTime().asSeconds();
    while (window->isOpen()) {
        float currTime = egg::getInstance().g_Clock.getElapsedTime().asSeconds();
        egg::getInstance().g_Delta = currTime - egg::getInstance().g_LastTime;
        egg::getInstance().g_LastTime = currTime;


        desktop.Update(egg::getInstance().g_Delta);

        sf::Event event;
        while (window->pollEvent(event)) {
            int handled = TwEventSFML23(&event);
            if (!handled) {
                p_input->allowCheck = true;
                switch (edMode) {
                case LuaMenu:
                    eventsLuaMenu(event);
                    break;
                case Idle:
                    eventsIdle(event);
                    break;
                case Fly:
                    eventsFly(event);
                    break;
                case Moving:
                    eventsMoving(event);
                    break;
                case Pulling:
                    eventsPulling(event);
                    break;
                }
                eventsAlways(event);
            } else { //if TwEventSFML handled
                p_input->allowCheck = false;
            }
            if (!p_input->cursorIsInsideWindow()) {
                desktop.HandleEvent(event);
            }
        }

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        if (p_input->isFocused()) {
            p_world->update();
            update();
            TwDraw();
        }
        glPopAttrib();

        window->pushGLStates();
        window->resetGLStates();
        glViewport(0, 0, window->getSize().x, window->getSize().y);
        sfgui.Display(*window);
        window->popGLStates();

        window->display();
    }

    TwTerminate();
    delete window;

    return EXIT_SUCCESS;
}

Editor::Editor()
{
    editorFlySpeed = 3.0f;
    edMode = Idle;
    selectedEntity = nullptr;
}

Editor::~Editor()
{
    delete p_world;
    delete p_input;
}

Editor::Ptr Editor::Create()
{
    egg::getInstance().g_Editor = true;
    Editor::Ptr ed(new Editor());
    return ed;
}

void Editor::registerLua()
{
    auto selectEntity = egg::getInstance().g_lua.CreateFunction<void(LuaUserdata<Entity>)>([&](LuaUserdata<Entity> eud) {
        Entity *pen = eud.GetPointer();
        if (selectedEntity != nullptr)
            selectedEntity->editorDesselect();
        selectedEntity = pen;
        selectedEntity->editorSelect();
        return;
    });
    auto registerAction = egg::getInstance().g_lua.CreateFunction<void(std::string, std::string, LuaFunction<void()>)>(
    [&](std::string name, std::string key, LuaFunction<void()> func) {
        actions[key[0]] = LuaAction(name, func);
        std::cout << "NEW ACTION" << std::endl;
    }
                          );
    auto registerCtrlAction = egg::getInstance().g_lua.CreateFunction<void(std::string, std::string, LuaFunction<void()>)>(
    [&](std::string name, std::string key, LuaFunction<void()> func) {
        ctrl_actions[key[0]] = LuaAction(name, func);
        std::cout << "NEW ACTION" << std::endl;
    }
                              );
    auto getCameraPos = egg::getInstance().g_lua.CreateFunction<LuaTable()>([&]() {
        auto table = egg::getInstance().g_lua.CreateTable();
        Camera *cam = p_world->pGraphics->getCamera();
        table.Set("x", cam->getPosition().x);
        table.Set("y", cam->getPosition().y);
        table.Set("z", cam->getPosition().z);
        std::cout << cam->getPosition().x << ":" << cam->getPosition().y << ":" << cam->getPosition().z << std::endl;
        return table;
    });
    auto setCameraPos = egg::getInstance().g_lua.CreateFunction<void(LuaTable)>([&](LuaTable table) {
        int x = table.Get<int>("x");
        int y = table.Get<int>("y");
        int z = table.Get<int>("z");
        Camera *cam = p_world->pGraphics->getCamera();
        cam->setPosition({x, y, z});
    });
    auto createPopup = egg::getInstance().g_lua.CreateFunction<LuaTable(string, LuaTable)>(
    [&](string name, LuaTable content) {
        auto result = egg::getInstance().g_lua.CreateTable();
        auto newpopup = sfg::Window::Create(sfg::Window::TOPLEVEL | sfg::Window::CLOSE);
        desktop.Add(newpopup);
        newpopup->SetTitle(name);
        auto box = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
        content.ForAllIntegerKeys([&](int num, LuaType::Value valueType) {
            LuaTable item = content.Get<LuaTable>(num);
            string type = item.Get<string>("item_type");
            if (type == "Button") {
                string but_text = item.Get<string>("text");
                auto button = sfg::Button::Create(but_text);
                std::shared_ptr<LuaAction> act(new LuaAction(but_text, item.Get<LuaFunction<void()>>("action")));
                button->GetSignal(sfg::Button::OnLeftClick).Connect([act, newpopup, this]() {
                    act->run();
                    newpopup->Show(false);
                    changeMode(Idle);
                });
                box->Pack(button);
            }
            if (type == "Label") {
                string label_text = item.Get<string>("text");
                auto label = sfg::Label::Create(label_text);
                box->Pack(label);
            }
            if (type == "EditLine") {
                string edit_text = item.Get<string>("text");
                string key = item.Get<string>("key");
                auto line = sfg::Entry::Create(edit_text);
                line->SetRequisition({150, 15});
                box->Pack(line);
                LuaFunction<string()> getText = egg::getInstance().g_lua.CreateFunction<string()>([line]() {
                    return line->GetText();
                });
                result.Set("get" + key, getText);
            }
        });
        newpopup->GetSignal(sfg::Window::OnMouseEnter).Connect([this]() {
            changeMode(Menu);
        });
        newpopup->GetSignal(sfg::Window::OnMouseLeave).Connect([this]() {
            changeMode(Idle);
        });
        newpopup->Add(box);
        newpopup->Show(true);
        changeMode(Menu);

        auto pos = sf::Mouse::getPosition(*window);
        newpopup->SetPosition({(float)pos.x, (float)pos.y});
        return result;
    });
    auto table = egg::getInstance().g_lua.CreateTable();
    table.Set("registerAction", registerAction);
    table.Set("registerCtrlAction", registerCtrlAction);
    table.Set("getCameraPos", getCameraPos);
    table.Set("setCameraPos", setCameraPos);
    table.Set("createPopup", createPopup);
    table.Set("selectEntity", selectEntity);
    egg::getInstance().g_lua.GetGlobalEnvironment().Set("Editor", table);
}

void Editor::loadAddons()
{
    registerLua();
    string scriptsdir = egg::getInstance().g_WorkingDir + "/plugins/";
    std::cout << scriptsdir << std::endl;
    auto scripts = GetFilesOfFormat(scriptsdir, "lua");
    std::cout << scripts.size() << " scripts found." << std::endl;
    for (auto script : scripts) {
        ifstream in;
        in.open(scriptsdir + script);
        stringstream buffer;
        buffer << in.rdbuf();
        in.close();
        std::cout << "Script loaded: " << buffer.str() << std::endl;
        std::cout << egg::getInstance().g_lua.RunScript(buffer.str()) << std::endl; // TODO: dangerous!
    }
}

void Editor::changeMode(EditorMode newMode)
{
    switch (newMode) {
    case Fly:
        edMode = Fly;
        modeStatus->SetText("Fly");
        break;
    case Idle:
        edMode = Idle;
        modeStatus->SetText("Idle");
        break;
    case LuaMenu:
        edMode = LuaMenu;
        modeStatus->SetText("Menu");
        break;
    case Pulling:
        edMode = Pulling;
        modeStatus->SetText("Pulling");
        break;
    case Moving:
        edMode = Moving;
        modeStatus->SetText("Moving");
        break;
    case Menu:
        edMode = Menu;
        modeStatus->SetText("Menu");
    }
}

RayCastInfo Editor::castRayScreen(bool fromCenter)
{
    sf::Vector2u sz = p_input->mainWindow->getSize();
    int screenWidth = sz.x, screenHeight = sz.y - topWndHeight;
    sf::Vector2i mp(screenWidth / 2, screenHeight / 2);
    if (!fromCenter) {
        mp = sf::Mouse::getPosition(*p_input->mainWindow);
        mp.y -= topWndHeight;
        mp.y = screenHeight - mp.y;
    }

    int mouseX = mp.x, mouseY = mp.y;

    glm::vec4 lRayStart_NDC(
        (float(mouseX) / float(screenWidth)  - 0.5f) * 2.0f,
        (float(mouseY) / float(screenHeight) - 0.5f) * 2.0f,
        -1.0,
        1.0f
    );
    glm::vec4 lRayEnd_NDC(
        (float(mouseX) / float(screenWidth)  - 0.5f) * 2.0f,
        (float(mouseY) / float(screenHeight) - 0.5f) * 2.0f,
        1.0,
        1.0f
    );

    WorldGraphics &graphics = *p_world->pGraphics;

    glm::mat4 M = glm::inverse(graphics.getCamera()->getProjectionMatrix() * graphics.getCamera()->getViewMatrix());
    glm::vec4 lRayStart_world = M * lRayStart_NDC;
    lRayStart_world /= lRayStart_world.w;
    glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ;
    lRayEnd_world  /= lRayEnd_world.w;

    glm::vec4 lrd4(lRayEnd_world - lRayStart_world);
    glm::vec3 lRayDir_world = glm::vec3(lrd4.x, lrd4.y, lrd4.z);
    glm::vec3 lRayOrigin_world = glm::vec3(lRayStart_world.x, lRayStart_world.y, lRayStart_world.z);

    return p_world->castRay(lRayOrigin_world, lRayDir_world);
}

void Editor::updateEntity(Entity *pen)
{
    // TODO: move from Entity class
}

void Editor::update()
{
    p_input->update();
    auto &physics = p_world->physics;
    auto &graphics = *p_world->pGraphics;
    Camera *cam = graphics.getCamera();

    physics.render(graphics.getCamera());
    cam->moveFree(vCameraSpeed * editorFlySpeed * egg::getInstance().g_Delta);

    if (selectedEntity != nullptr) {
        selectedEntity->editorUpdate();
    }

    if (p_input->lockMouse) {
        cam->rotate(0.1f * egg::getInstance().g_Delta * p_input->mouseDelta.x,
                    0.1f * egg::getInstance().g_Delta * p_input->mouseDelta.y);
    } else {
        if (p_input->cursorIsInsideWindow()) {
            if (edMode == Pulling) {
                if (selectedEntity == NULL) {
                    changeMode(Idle);
                    return;
                }
                sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow);
                glm::vec3 camPos = graphics.getCamera()->getPosition();
                camPos = camPos - selectedEntity->position;
                camPos *= editorFlySpeed * ((mpos.y - mposOffsetMoving.y) / (1.0f * p_input->windowSize.y)) / glm::length(camPos);
                selectedEntity->position += camPos;
                mposOffsetMoving = glm::vec2(mpos.x, mpos.y);
            }
            if (edMode == Moving) {
                if (selectedEntity == NULL) {
                    changeMode(Idle);
                    return;
                }
                glm::mat4 camMat = graphics.getCamera()->getProjectionMatrix() * graphics.getCamera()->getViewMatrix();
                glm::vec4 entPos = glm::vec4(selectedEntity->position.x, selectedEntity->position.y, selectedEntity->position.z, 1.0f);
                entPos =  camMat * entPos;
                sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow);
                mpos.y = p_input->windowSize.y - mpos.y;
                glm::vec4 endPosNDC = glm::vec4((float(mpos.x) / float(p_input->windowSize.x)) * 2.0f - 1.0f + mposOffsetMoving.x,
                                                (float(mpos.y) / float(p_input->windowSize.y)) * 2.0f - 1.0f + mposOffsetMoving.y,
                                                entPos.z / entPos.w,
                                                1.0f);
                glm::vec4 endPosWLD = glm::inverse(camMat) * endPosNDC;
                endPosWLD /= endPosWLD.w;
                selectedEntity->position = glm::vec3(endPosWLD.x, endPosWLD.y, endPosWLD.z);
            }
        }
    }
}
