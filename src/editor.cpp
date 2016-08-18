#include <string>
#include <istream>
#include <GL/glew.h>
#include <SFGUI/RenderQueue.hpp>
#include <SFGUI/Renderers.hpp>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "dialogs/tinyfiledialogs.h"
#include "world.h"
#include "input_handler.h"
#include "editor.h"
#include "entity.h"
#include "global.h"
#include "editorGUI.h"
#include "dirtools.h"
#include "properties.h"

class ColorButton : public sfg::Button
{
public:
    typedef std::shared_ptr<ColorButton> Ptr;
    static Ptr Create(glm::vec3 c)
    {
        Ptr cb(new ColorButton(c));
        return cb;
    }

    void SetFillColor(glm::vec3 c) { _col = c; Invalidate(); }

private:
    ColorButton(glm::vec3 c) : _col(c) {}

    std::unique_ptr<sfg::RenderQueue> InvalidateImpl() const override
    {
        std::unique_ptr<sfg::RenderQueue> queue(new sfg::RenderQueue);

        queue->Add(
            sfg::Renderer::Get().CreatePane(
                sf::Vector2f( 0.f, 0.f ),
                sf::Vector2f( GetAllocation().width, GetAllocation().height ),
                1.0f,
                sf::Color(_col.x*255, _col.y*255, _col.z*255),
                sf::Color(0),
                20.f
            )
        );

        return queue;
    }

    glm::vec3 _col;
};

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
    selectEntity(nullptr);
    p_world->Clear();
    currentWorld = "";
}

void Editor::Load()
{
    std::string path;
    const char *result = tinyfd_openFileDialog("Load world", "./", 0, NULL, 0);
    if (result) {
        path = relativePath(result);
        p_world->Love(path);
        currentWorld = path;
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

void Editor::Save()
{
    if(currentWorld.empty())
    {
        SaveAs();
        return;
    }
    p_world->Save(currentWorld);
}

void Editor::resizeGUIComponents(unsigned width, unsigned height)
{
    leftWindow->SetAllocation(sf::FloatRect(0.0f, static_cast<float>(topWndHeight),
                                            static_cast<float>(leftWndWidth), static_cast<float>(height - topWndHeight)));

    topWindow->SetAllocation(sf::FloatRect(0.0f, 0.0f,
                                           static_cast<float>(width), static_cast<float>(topWndHeight)));

    egg::getInstance().g_DrawOrigin.x = leftWndWidth;
    egg::getInstance().g_DrawOrigin.y = dwnWndHeight;

    egg::getInstance().g_Resolution.x = width - leftWndWidth - rghtWndWidth;
    egg::getInstance().g_Resolution.y = height - topWndHeight - dwnWndHeight;
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
        default: break;
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
        break;
    default: break;
    }
}

void Editor::eventsMenu(sf::Event &event)
{

}

void Editor::eventsIdle(sf::Event &event)
{
    switch (event.type) {

        case sf::Event::KeyReleased: {
            if (p_input->keyPressed(sf::Keyboard::LControl))
            {
                if (event.key.code == sf::Keyboard::C) {
                    copyEntity();
                } else if (event.key.code == sf::Keyboard::V) {
                    pasteEntity();
                }
            }
            break;
        }

        case sf::Event::KeyPressed: {
            if (event.key.code == sf::Keyboard::Escape) {
                p_input->switchLockMouse(true);
                changeMode(Fly);
                return;
            }
            if (p_input->cursorIsInsideWindow()) {
                if (event.key.code == sf::Keyboard::Tilde) {
                    selectEntity(nullptr);
                    return;
                }

                if (event.key.code == sf::Keyboard::Delete) {
                    if (selectedEntity) {
                        p_world->removeEntity(selectedEntity);
                    }
                    selectEntity(nullptr);
                    return;
                }
            }
            break;
        }

        case sf::Event::MouseButtonPressed: {

            if (event.mouseButton.button == sf::Mouse::Right) {

                if (selectedEntity && p_input->keyPressed(sf::Keyboard::LControl)) {
                    changeMode(Pulling);
                    sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow);
                    mposOffsetMoving = glm::vec2(mpos.x, mpos.y);
                    return;
                }

            } else if (event.mouseButton.button == sf::Mouse::Left){

                auto &graphics = *p_world->pGraphics;

                float tmNow = egg::getInstance().g_Clock.getElapsedTime().asSeconds();

                if (selectedEntity && p_input->keyPressed(sf::Keyboard::LControl)) {
                    if (tmNow - doubleClickTime <= 0.5f && (moveModePeriod < 0.1f || tmNow - moveModeTime - moveModePeriod >= 1.0f)) {
                        RayCastInfo ri = castRayScreen();
                        if (ri.enHit) {
                            selectedEntity->setProperty("Position", ri.posHit);
                        } else {
                            selectedEntity->setProperty("Position", ri.posOrigin + ri.direction*5.0f);
                        }
                        doubleClickTime -= 10.0f;
                        return;
                    }
                    doubleClickTime = tmNow;

                    changeMode(Moving);
                    moveModeTime = tmNow;
                    sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow) - sf::Vector2i(leftWndWidth + rghtWndWidth, topWndHeight + dwnWndHeight);
                    sf::Vector2u sz = p_input->mainWindow->getSize();
                    mpos.y = sz.y - mpos.y;
                    glm::mat4 camMat = graphics.getCamera()->getProjectionMatrix() * graphics.getCamera()->getViewMatrix();
                    const glm::vec3& enPosV3 = selectedEntity->getProperty<glm::vec3>("Position");
                    glm::vec4 entPos = glm::vec4(enPosV3.x, enPosV3.y, enPosV3.z, 1.0f);
                    entPos =  camMat * entPos;
                    entPos /= entPos.w;
                    int screenWidth = sz.x - leftWndWidth - rghtWndWidth,
                        screenHeight = sz.y - topWndHeight - dwnWndHeight;
                    mposOffsetMoving = glm::vec2(entPos.x - (float(mpos.x) / float(screenWidth)) * 2.0f + 1.0f,
                                                 entPos.y - (float(mpos.y) / float(screenHeight)) * 2.0f + 1.0f);
                    return;

                } else {

                    if(tmNow - doubleClickTime < 0.5f) return;

                    RayCastInfo rci = castRayScreen();
                    if (rci.enHit) {
                        if (selectedEntity != rci.enHit) {
                            selectEntity(rci.enHit);
                        }
                    } else {
                        selectEntity(nullptr);
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
            }
            break;
        }
        default: break;
    }
}

void Editor::eventsMoving(sf::Event &event)
{
    switch (event.type) {
        case sf::Event::MouseButtonReleased:
            if (event.mouseButton.button == sf::Mouse::Left) {
                moveModePeriod = egg::getInstance().g_Clock.getElapsedTime().asSeconds() - moveModeTime;
                changeMode(Idle);
            }
            break;
        default: break;
    }
}

void Editor::eventsPick(sf::Event &event)
{
    switch (event.type) {
        case sf::Event::KeyReleased:
            if (event.key.code == sf::Keyboard::Escape) {
                changeMode(Idle);
            }
            break;

        case sf::Event::MouseButtonPressed:
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                RayCastInfo rci = castRayScreen();

                assert(selectedEntity && !pointerProp.empty());
                selectedEntity->setProperty<EntityPointer>(pointerProp, EntityPointer(rci.enHit));
                changeMode(Idle);
                pointerProp = "";
            }
            break;
        default: break;
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
        default: break;
    }
}

void Editor::eventsFly(sf::Event &event)
{
    switch (event.type) {
        case sf::Event::KeyPressed: {
            switch (event.key.code) {
                case sf::Keyboard::Up:
                case sf::Keyboard::W: {
                    vCameraSpeed.z = 1;
                    break;
                }
                case sf::Keyboard::Down:
                case sf::Keyboard::S: {
                    vCameraSpeed.z = -1;
                    break;
                }
                case sf::Keyboard::Right:
                case sf::Keyboard::D: {
                    vCameraSpeed.x = 1;
                    break;
                }
                case sf::Keyboard::Left:
                case sf::Keyboard::A: {
                    vCameraSpeed.x = -1;
                    break;
                }
                case sf::Keyboard::Space: {
                    vCameraSpeed.y = 1;
                    break;
                }
                case sf::Keyboard::C: {
                    vCameraSpeed.y = -1;
                    break;
                }
                case sf::Keyboard::Escape: {
                    changeMode(Idle);
                    p_input->switchLockMouse(false);
                    vCameraSpeed = { 0, 0, 0 };
                    break;
                }
                default: break;
            }   // event.key.code
            break;
        }   // case sf::Event::KeyPressed
        case sf::Event::KeyReleased: {
            switch (event.key.code) {
                case sf::Keyboard::Up:
                case sf::Keyboard::W: {
                    vCameraSpeed.z = 0;
                    break;
                }
                case sf::Keyboard::Down:
                case sf::Keyboard::S: {
                    vCameraSpeed.z = 0;
                    break;
                }
                case sf::Keyboard::Right:
                case sf::Keyboard::D: {
                    vCameraSpeed.x = 0;
                    break;
                }
                case sf::Keyboard::Left:
                case sf::Keyboard::A: {
                    vCameraSpeed.x = 0;
                    break;
                }
                case sf::Keyboard::Space: {
                    vCameraSpeed.y = 0;
                    break;
                }
                case sf::Keyboard::C: {
                    vCameraSpeed.y = 0;
                    break;
                }
                default: break;
            }   // event.key.code
            break;
        }       // case sf::Event::KeyReleased
        case sf::Event::MouseWheelScrolled: {
            double delta = event.mouseWheelScroll.delta;
            if (delta > 0)       editorFlySpeed = glm::min(100.0f, editorFlySpeed * 2.0f);
            else if (delta < 0)  editorFlySpeed = glm::max(0.25f, editorFlySpeed * 0.5f);
            break;
        }       // case sf::Event::MouseWheelScrolled
        default: break;
    }           // event.type
}

void Editor::fillActionsMenu()
{
    auto box = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
    std::unordered_map<std::string, char> names;
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

void Editor::copyEntity()
{
    copyEntitySerialized = "";
    if(!selectedEntity) return;

    rapidjson::Document doc;
    rapidjson::Value out;
    out.SetArray();
    rapidjson::Value val = selectedEntity->SerializeForCopying(doc);
    rapidjson::Value classname;
    std::string strClass = selectedEntity->getClass();
    classname.SetString(strClass.c_str(), strClass.length(), doc.GetAllocator());
    val.AddMember("class", classname, doc.GetAllocator());
    out.PushBack(val, doc.GetAllocator());
    rapidjson::StringBuffer strbuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
    out.Accept(writer);
    copyEntitySerialized = strbuf.GetString();
}

void Editor::pasteEntity()
{
    if(copyEntitySerialized.empty()) return;

    Entity* paste = p_world->Paste(copyEntitySerialized);
    RayCastInfo rci = castRayScreen();
    if(rci.enHit != nullptr)
    {
        paste->setPosition(rci.posHit);
    }
    selectEntity(paste);
}

void Editor::setupGUI()
{
    leftWindow = sfg::Window::Create(sfg::Window::Style::BACKGROUND);
    topWindow = sfg::Window::Create(sfg::Window::Style::BACKGROUND);
    //fill windows here

    /**********************LEFT BOX*************************/

    auto tableEntities = sfg::Table::Create();
    tableEntities->Attach(sfg::Separator::Create(),
                          sf::Rect<sf::Uint32>(0,0,2,1),
                          sfg::Table::FILL,
                          0);

    unsigned freeRow = 1u;
    std::vector<std::string> classes = Incubator::getRegisteredClasses();
    std::sort(classes.begin(), classes.end());

    sf::Image iconAdd;
    bool setIcon = iconAdd.loadFromFile("./gui/add.png");

    for (std::string &cl : classes) {
        auto btn = sfg::Button::Create();
        btn->GetSignal(sfg::Widget::OnLeftClick).Connect([this, cl]() { selectEntity(p_world->createEntity(cl)); });

        auto align = sfg::Alignment::Create();
        align->Add(sfg::Label::Create(cl));
        align->SetScale(sf::Vector2f(0.0f, 0.0f));
        align->SetAlignment(sf::Vector2f(0.0f, 0.0f));

        if(setIcon) {
            btn->SetImage(sfg::Image::Create(iconAdd));
        }

        tableEntities->Attach(align,
                              sf::Rect<sf::Uint32>(0,freeRow,1,1),
                              sfg::Table::FILL | sfg::Table::EXPAND,
                              0);
        tableEntities->Attach(btn,
                              sf::Rect<sf::Uint32>(1,freeRow,1,1),
                              0,
                              0);
        tableEntities->Attach(sfg::Separator::Create(),
                              sf::Rect<sf::Uint32>(0,++freeRow,2,1),
                              sfg::Table::FILL,
                              0);
    }

    tableEntityProperties = sfg::Table::Create();

    auto leftTabs = sfg::Notebook::Create();
    leftTabs->SetTabPosition(sfg::Notebook::TabPosition::BOTTOM);
    leftTabs->AppendPage(tableEntities, sfg::Label::Create("Entities"));
    leftTabs->AppendPage(tableEntityProperties, sfg::Label::Create("Properties"));

    leftWindow->Add(leftTabs);
    leftWindow->Show(true);

    /**********************TOP BOX*************************/

    auto topBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 2.0f);

    auto btnNew = sfg::Button::Create("New World");
    auto btnLoad = sfg::Button::Create("Load World");
    auto btnSaveAs = sfg::Button::Create("Save As");
    auto btnSave = sfg::Button::Create("Save");

    btnNew->GetSignal(sfg::Widget::OnLeftClick).Connect(std::bind(&Editor::NewWorld, this));
    btnLoad->GetSignal(sfg::Widget::OnLeftClick).Connect(std::bind(&Editor::Load, this));
    btnSaveAs->GetSignal(sfg::Widget::OnLeftClick).Connect(std::bind(&Editor::SaveAs, this));
    btnSave->GetSignal(sfg::Widget::OnLeftClick).Connect(std::bind(&Editor::Save, this));

    modeStatus = sfg::Label::Create("Idle");
    popup = sfg::Window::Create(sfg::Window::TOPLEVEL);
    popup->Show(false);

    topBox->Pack(btnNew, false, true);
    topBox->Pack(btnLoad, false, true);
    topBox->Pack(btnSaveAs, false, true);
    topBox->Pack(btnSave, false, true);
    topBox->Pack(modeStatus, false, true);

    topWindow->Add(topBox);


    //~fill windows here
    desktop.Add(leftWindow);
    desktop.Add(topWindow);
    desktop.Add(popup);
    resizeGUIComponents(window->getSize().x, window->getSize().y);
    desktop.Update(0.f);
}

int Editor::run()
{
    printf("%s",egg::getInstance().logo.c_str());

    sf::ContextSettings cs;
    cs.antialiasingLevel = 4;
    cs.depthBits = 24;
    cs.majorVersion = 3;
    cs.minorVersion = 0;
    cs.stencilBits = 8;

    window = new sf::RenderWindow(sf::VideoMode(egg::getInstance().g_Resolution.x, egg::getInstance().g_Resolution.y), "Eggine Editor", sf::Style::Default, cs);
    window->setVerticalSyncEnabled(true);
    window->setActive();
    window->setKeyRepeatEnabled(false);

    glewExperimental = true;
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    p_input = new InputHandler(window);
    p_world = new World();
    egg::getInstance().g_World = p_world;

    sfg::SFGUI sfgui;
    window->setActive();
    desktop.LoadThemeFromFile("./gui/eggine.theme");

    setupGUI();

    loadAddons();

    if (!startupWorld.empty()) {
        currentWorld = startupWorld;
        p_world->Love(startupWorld);
    }

    egg::getInstance().g_Clock.restart();
    doubleClickTime = 0.0f;
    moveModeTime = 0.0f;
    moveModePeriod = 1.0f;

    while (window->isOpen()) {

        float currTime = egg::getInstance().g_Clock.getElapsedTime().asSeconds();
        egg::getInstance().g_Delta = currTime - egg::getInstance().g_LastTime;
        egg::getInstance().g_LastTime = currTime;

        desktop.Update(egg::getInstance().g_Delta);

        sf::Event event;
        while (window->pollEvent(event)) {

            p_input->allowCheck = true;

            if(p_input->cursorIsInsideWindow()) {
                switch (edMode) {
                    case LuaMenu:
                        eventsLuaMenu(event);
                        desktop.HandleEvent(event);
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
                    case PickEntity:
                        eventsPick(event);
                        break;
                    case Menu:
                        desktop.HandleEvent(event);
                        break;
                    default:
                        assert(false);
                }
            } else {
                desktop.HandleEvent(event);
            }

            eventsAlways(event);
        }

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        if (p_input->isFocused()) {
            p_world->update();
            update();
            drawGrid();
        }
        glPopAttrib();

        window->pushGLStates();
        glViewport(0, 0, window->getSize().x, window->getSize().y);
        sfgui.Display(*window);
        window->popGLStates();

        window->display();
    }

    delete window;

    return EXIT_SUCCESS;
}

void Editor::selectEntity(Entity* en)
{
    static std::vector<std::function<void(void)>> refreshCallbacks = {};

    if(selectedEntity) {
        selectedEntity->setSelected(false);
    }

    pointerProp = "";
    tableEntityProperties->RemoveAll();
    refreshCallbacks.clear();
    selectedEntity = en;

    if(!selectedEntity) return;

    selectedEntity->setSelected(true);

    /**************** FILL GUI WITH PROPERTIES **************************/

    sf::Image iconPick;
    bool setIcon = iconPick.loadFromFile("./gui/pick.png");

    tableEntityProperties->GetSignal(sfg::Widget::OnMouseEnter).Connect(
                    []() { for(auto &cb : refreshCallbacks) cb(); });

    const Entity::Props& props = selectedEntity->getProperties();

    unsigned freeRow = 0;
    for(auto it=props.begin(); it!=props.end(); it++)
    {
        const Property& prop = it->second;
        const std::string propName = it->first;

        tableEntityProperties->Attach(sfg::Separator::Create(sfg::Separator::Orientation::VERTICAL),
                                      sf::Rect<sf::Uint32>(1,freeRow,1,1),
                                      0,
                                      sfg::Table::FILL);

        /**************** COLOR ****************************************************/
        if(prop.m_tid == typeid(Color).hash_code())
        {
            const Color& col = *static_cast<Color*>(prop.m_data);
            auto colButton = ColorButton::Create(col.value);
            colButton->SetRequisition( sf::Vector2f( 100.0f, 20.0f ) );
            ColorButton* colButtonPTR = colButton.get();

            colButton->GetSignal(sfg::Widget::OnLeftClick).Connect(
                            [en, propName, colButtonPTR]()
                            {
                                glm::vec3 col = (en->getProperty<Color>(propName)).value;
                                unsigned char startColor[3];
                                startColor[0] = col.x*255;
                                startColor[1] = col.y*255;
                                startColor[2] = col.z*255;
                                tinyfd_colorChooser("Pick color", NULL, startColor, startColor);
                                col.x = startColor[0]/255.0f;
                                col.y = startColor[1]/255.0f;
                                col.z = startColor[2]/255.0f;
                                en->setProperty<Color>(propName, Color(col));
                                colButtonPTR->SetFillColor(col);
                            });

            tableEntityProperties->Attach(colButton,
                                          sf::Rect<sf::Uint32>(2,freeRow,1,1),
                                          sfg::Table::FILL | sfg::Table::EXPAND,
                                          0);

        /**************** ENTITYPOINTER ****************************************************/
        } else if(prop.m_tid == typeid(EntityPointer).hash_code())
        {
            const EntityPointer& ep = *static_cast<EntityPointer*>(prop.m_data);
            std::string descr = "(none)";
            if(ep)
            {
                descr = std::to_string(ep->getMultipass()) + ": " + ep->getProperty<std::string>("Name");
            }

            auto pButton = sfg::Button::Create("<");
            auto pView = sfg::Viewport::Create();
            pView->SetRequisition( sf::Vector2f( 100.0f, 20.0f ) );
            auto pLabel = sfg::Label::Create(descr);
            sfg::Label* pLabelPTR = pLabel.get();
            pLabel->SetRequisition( sf::Vector2f( 20.0f, 20.0f ) );
            pView->Add(pLabel);
            auto pBox = sfg::Box::Create();
            pBox->Pack(pView, true, true);
            pBox->Pack(pButton, false, true);

            if(setIcon) {
                pButton->SetImage(sfg::Image::Create(iconPick));
                pButton->SetLabel("");
            }

            refreshCallbacks.push_back(
                    [en, propName, pLabelPTR]()
                    {
                        const EntityPointer& _ep = en->getProperty<EntityPointer>(propName);
                        std::string _descr = "(none)";
                        if(_ep)
                        {
                            _descr = std::to_string(_ep->getMultipass()) + ": " + _ep->getProperty<std::string>("Name");
                        }
                        pLabelPTR->SetText(_descr);
                    });

            pButton->GetSignal(sfg::Widget::OnLeftClick).Connect(
                     [propName, this]()
                     {
                         pointerProp = propName;
                         changeMode(PickEntity);
                     });

            auto align = sfg::Alignment::Create();
            align->Add(pBox);
            align->SetAlignment(sf::Vector2f(1.0f, 0.0f));
            align->SetScale(sf::Vector2f(1.0f, 0.0f));
            tableEntityProperties->Attach(align,
                                          sf::Rect<sf::Uint32>(2,freeRow,1,1),
                                          sfg::Table::FILL | sfg::Table::EXPAND,
                                          0);

        /**************** INT ****************************************************/
        } else if(prop.m_tid == typeid(int).hash_code())
        {
            auto v = sfg::SpinButton::Create(-9999999.0f, 999999999999.0f, 1.0f);
            v->SetDigits(0);
            v->SetRequisition(sf::Vector2f(100.0f, 20.0f));
            int val = *static_cast<int*>(prop.m_data);
            v->SetValue((float)val);

            sfg::SpinButton* vPTR = v.get();
            refreshCallbacks.push_back([en, propName, vPTR]()
                            {
                                int v = en->getProperty<int>(propName);
                                vPTR->SetValue((float)v);
                            });
            v->GetSignal(sfg::SpinButton::OnValueChanged).Connect(
                            [en, propName, vPTR]()
                            {
                                en->setProperty<int>(propName, (int)(vPTR->GetValue()));
                            });

            auto align = sfg::Alignment::Create();
            align->Add(v);
            align->SetAlignment(sf::Vector2f(1.0f, 0.0f));
            align->SetScale(sf::Vector2f(1.0f, 0.0f));
            tableEntityProperties->Attach(align,
                                          sf::Rect<sf::Uint32>(2,freeRow,1,1),
                                          sfg::Table::FILL | sfg::Table::EXPAND,
                                          0);

        /**************** FLOAT ****************************************************/
        } else if(prop.m_tid == typeid(float).hash_code())
        {
            auto v = sfg::SpinButton::Create(-9999999.0f, 999999999999.0f, 0.01f);
            v->SetDigits(2);
            v->SetRequisition(sf::Vector2f(100.0f, 20.0f));
            float val = *static_cast<float*>(prop.m_data);
            v->SetValue(val);

            sfg::SpinButton* vPTR = v.get();
            refreshCallbacks.push_back([en, propName, vPTR]()
                            {
                                float v = en->getProperty<float>(propName);
                                vPTR->SetValue(v);
                            });
            v->GetSignal(sfg::SpinButton::OnValueChanged).Connect(
                            [en, propName, vPTR]()
                            {
                                en->setProperty<float>(propName, vPTR->GetValue());
                            });

            auto align = sfg::Alignment::Create();
            align->Add(v);
            align->SetAlignment(sf::Vector2f(1.0f, 0.0f));
            align->SetScale(sf::Vector2f(1.0f, 0.0f));
            tableEntityProperties->Attach(align,
                                          sf::Rect<sf::Uint32>(2,freeRow,1,1),
                                          sfg::Table::FILL | sfg::Table::EXPAND,
                                          0);

        /**************** FILENAME ****************************************************/
        } else if(prop.m_tid == typeid(FileName).hash_code())
        {
            FileName fn = *static_cast<FileName*>(prop.m_data);
            auto fnBox = sfg::Box::Create();
            auto fnEntry = sfg::Label::Create(fn.path);
            auto fnButton = sfg::Button::Create("<");
            auto fnView = sfg::Viewport::Create();
            fnView->SetRequisition( sf::Vector2f( 100.0f, 20.0f ) );
            fnView->Add(fnEntry);

            if(setIcon) {
                fnButton->SetImage(sfg::Image::Create(iconPick));
                fnButton->SetLabel("");
            }

            fnEntry->SetRequisition(sf::Vector2f(20.0f, 20.0f));
            sfg::Label* fnEntryPTR = fnEntry.get();

            fnButton->GetSignal(sfg::Widget::OnLeftClick).Connect(
                      [en, propName, fnEntryPTR]()
                      {
                          const char* filter[] = { (propName.find("Model") != std::string::npos ? "*.mconf" : "*.*") };
                          const char* result = tinyfd_openFileDialog("Select Resource","./",1,filter,0);
                          if(result)
                          {
                              std::string path = relativePath(result);
                              en->setProperty<FileName>(propName, FileName(path));
                              fnEntryPTR->SetText(en->getProperty<FileName>(propName).path);
                          }
                      });

            fnBox->Pack(fnView, true, true);
            fnBox->Pack(fnButton, false, true);

            auto align = sfg::Alignment::Create();
            align->Add(fnBox);
            align->SetAlignment(sf::Vector2f(1.0f, 0.0f));
            align->SetScale(sf::Vector2f(1.0f, 0.0f));
            tableEntityProperties->Attach(align,
                                          sf::Rect<sf::Uint32>(2,freeRow,1,1),
                                          sfg::Table::FILL | sfg::Table::EXPAND,
                                          0);

        /**************** VEC3 ****************************************************/
        } else if(prop.m_tid == typeid(glm::vec3).hash_code())
        {
            auto vecBox = sfg::Box::Create();
            auto v1 = sfg::SpinButton::Create(-9999999.0f, 999999999999.0f, 0.5f);
            auto v2 = sfg::SpinButton::Create(-9999999.0f, 999999999999.0f, 0.5f);
            auto v3 = sfg::SpinButton::Create(-9999999.0f, 999999999999.0f, 0.5f);
            v1->SetDigits(1);
            v2->SetDigits(1);
            v3->SetDigits(1);
            v1->SetRequisition(sf::Vector2f(30.0f, 20.0f));
            v2->SetRequisition(sf::Vector2f(30.0f, 20.0f));
            v3->SetRequisition(sf::Vector2f(30.0f, 20.0f));
            glm::vec3 val = *static_cast<glm::vec3*>(prop.m_data);
            v1->SetValue(val.x);
            v2->SetValue(val.y);
            v3->SetValue(val.z);
            sfg::SpinButton* v1PTR = v1.get();
            sfg::SpinButton* v2PTR = v2.get();
            sfg::SpinButton* v3PTR = v3.get();
            refreshCallbacks.push_back([en, propName, v1PTR, v2PTR, v3PTR]()
                            {
                                glm::vec3 v = en->getProperty<glm::vec3>(propName);
                                v1PTR->SetValue(v.x);
                                v2PTR->SetValue(v.y);
                                v3PTR->SetValue(v.z);
                            });
            auto callb = [en, propName, v1PTR, v2PTR, v3PTR]()
                            {
                                en->setProperty<glm::vec3>(propName, glm::vec3(v1PTR->GetValue(), v2PTR->GetValue(), v3PTR->GetValue()));
                            };
            v1->GetSignal(sfg::SpinButton::OnValueChanged).Connect(callb);
            v2->GetSignal(sfg::SpinButton::OnValueChanged).Connect(callb);
            v3->GetSignal(sfg::SpinButton::OnValueChanged).Connect(callb);
            vecBox->Pack(v1, true, true);
            vecBox->Pack(v2, true, true);
            vecBox->Pack(v3, true, true);

            auto align = sfg::Alignment::Create();
            align->Add(vecBox);
            align->SetAlignment(sf::Vector2f(1.0f, 0.0f));
            align->SetScale(sf::Vector2f(1.0f, 0.0f));
            tableEntityProperties->Attach(align,
                                          sf::Rect<sf::Uint32>(2,freeRow,1,1),
                                          sfg::Table::FILL | sfg::Table::EXPAND,
                                          0);

        /**************** BOOL ****************************************************/
        } else if(prop.m_tid == typeid(bool).hash_code())
        {
            auto checkb = sfg::CheckButton::Create("");
            checkb->SetRequisition(sf::Vector2f(0.0f, 20.0f));
            checkb->SetActive(*static_cast<bool*>(prop.m_data));
            sfg::CheckButton* checkbPTR = checkb.get();
            checkb->GetSignal(sfg::CheckButton::OnToggle).Connect(
                    [en, propName, checkbPTR]()
                        {
                            en->setProperty<bool>(propName, checkbPTR->IsActive());
                        });

            auto align = sfg::Alignment::Create();
            align->Add(checkb);
            align->SetAlignment(sf::Vector2f(1.0f, 0.0f));
            align->SetScale(sf::Vector2f(0.0f, 0.0f));
            tableEntityProperties->Attach(align,
                                          sf::Rect<sf::Uint32>(2,freeRow,1,1),
                                          sfg::Table::FILL | sfg::Table::EXPAND,
                                          0);

        /**************** STRING ****************************************************/
        } else if(prop.m_tid == typeid(std::string).hash_code())
        {
            auto entry = sfg::Entry::Create(*static_cast<std::string*>(prop.m_data));
            entry->SetRequisition(sf::Vector2f(100.0f, 0.0f));
            sfg::Entry* entryPTR = entry.get();
            entry->GetSignal(sfg::Entry::OnTextChanged).Connect(
                    [en, propName, entryPTR]()
                        {
                            en->setProperty<std::string>(propName, std::string(entryPTR->GetText().toAnsiString()));
                        });
            tableEntityProperties->Attach(entry,
                                          sf::Rect<sf::Uint32>(2,freeRow,1,1),
                                          sfg::Table::FILL | sfg::Table::EXPAND,
                                          0);
        } else {
            std::cout << "Unknown property with name '" << propName << "' with tid " << prop.m_tid << "\n";
            assert(false);
        }


        auto pView = sfg::Viewport::Create();
        pView->SetRequisition( sf::Vector2f( 100.0f, 20.0f ) );
        pView->Add(sfg::Label::Create(propName));
        auto align = sfg::Alignment::Create();
        align->Add(pView);
        align->SetAlignment(sf::Vector2f(0.0f, 0.0f));
        align->SetScale(sf::Vector2f(0.0f, 0.0f));

        tableEntityProperties->Attach(align,
                                      sf::Rect<sf::Uint32>(0,freeRow,1,1),
                                      sfg::Table::FILL | sfg::Table::EXPAND,
                                      0);
        tableEntityProperties->Attach(sfg::Separator::Create(),
                                      sf::Rect<sf::Uint32>(0,++freeRow,3,1),
                                      sfg::Table::FILL,
                                      0);
    }

    tableEntityProperties->Attach(sfg::Separator::Create(sfg::Separator::Orientation::VERTICAL),
                                  sf::Rect<sf::Uint32>(1,freeRow+1,1,1),
                                  0,
                                  sfg::Table::FILL | sfg::Table::EXPAND);
}

Editor::Editor()
{
    currentWorld = "";
    pointerProp = "";
    copyEntitySerialized = "";
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

        this->selectEntity(pen);
        return;
    });
    auto registerAction = egg::getInstance().g_lua.CreateFunction<void(std::string, std::string, LuaFunction<void()>)>(
    [&](std::string name, std::string key, LuaFunction<void()> func) {
        actions[key[0]] = LuaAction(name, func);
    }
                          );
    auto registerCtrlAction = egg::getInstance().g_lua.CreateFunction<void(std::string, std::string, LuaFunction<void()>)>(
    [&](std::string name, std::string key, LuaFunction<void()> func) {
        ctrl_actions[key[0]] = LuaAction(name, func);
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
    auto createPopup = egg::getInstance().g_lua.CreateFunction<LuaTable(std::string, LuaTable)>(
    [&](std::string name, LuaTable content) {
        auto result = egg::getInstance().g_lua.CreateTable();
        auto newpopup = sfg::Window::Create(sfg::Window::TOPLEVEL | sfg::Window::CLOSE);
        std::weak_ptr<sfg::Window> weakNewpopup = newpopup;
        desktop.Add(newpopup);
        newpopup->SetTitle(name);
        newpopup->GetSignal(newpopup->OnCloseButton).Connect([newpopup, this] { 
            newpopup->RemoveAll();
            newpopup->Show(false);
            desktop.Remove(newpopup);
            //desktop.Refresh();
            changeMode(Idle);
        });
        
        auto box = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
        content.ForAllIntegerKeys([&](int num, LuaType::Value valueType) {
            LuaTable item = content.Get<LuaTable>(num);
            std::string type = item.Get<std::string>("item_type");
            if (type == "Button") {
                std::string but_text = item.Get<std::string>("text");
                auto button = sfg::Button::Create(but_text);
                std::shared_ptr<LuaAction> act(new LuaAction(but_text, item.Get<LuaFunction<void()>>("action")));
                button->GetSignal(sfg::Button::OnLeftClick).Connect([act, weakNewpopup, this]() {
                    act->run();
                    if(auto npp = weakNewpopup.lock())
                        desktop.Remove(npp);
                    changeMode(Idle);
                });
                box->Pack(button);
            }
            if (type == "Label") {
                std::string label_text = item.Get<std::string>("text");
                auto label = sfg::Label::Create(label_text);
                box->Pack(label);
            }
            if (type == "EditLine") {
                std::string edit_text = item.Get<std::string>("text");
                std::string key = item.Get<std::string>("key");
                auto line = sfg::Entry::Create(edit_text);
                line->SetRequisition({150, 15});
                box->Pack(line);
                std::weak_ptr<sfg::Entry> lineWeak = line;
                LuaFunction<std::string()> getText = egg::getInstance().g_lua.CreateFunction<std::string()>([lineWeak]() {
                    if(auto lineSP = lineWeak.lock())
                        return lineSP->GetText().toAnsiString();
                    return std::string("");
                });
                result.Set("get" + key, getText);
            }
        });
        /*
        newpopup->GetSignal(sfg::Window::OnMouseEnter).Connect([this]() {
            changeMode(Menu);
        });
        newpopup->GetSignal(sfg::Window::OnMouseLeave).Connect([this]() {
            changeMode(Idle);
        });
        */
        newpopup->GetSignal(sfg::Window::OnCloseButton).Connect([this, weakNewpopup]()
        {
            if(auto npp = weakNewpopup.lock())
                desktop.Remove(npp);
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
    std::string scriptsdir = egg::getInstance().g_WorkingDir + "/plugins/";
    std::cout << scriptsdir << std::endl;
    auto scripts = GetFilesOfFormat(scriptsdir, "lua");
    std::cout << scripts.size() << " scripts found." << std::endl;
    for (auto &script : scripts) {
        std::ifstream in;
        in.open(scriptsdir + script);
        std::stringstream buffer;
        buffer << in.rdbuf();
        in.close();
        std::cout << egg::getInstance().g_lua.RunScript(buffer.str()) << std::endl; // TODO: dangerous!
    }
}

void Editor::changeMode(EditorMode newMode)
{
    switch (newMode) {
        case PickEntity:
            edMode = PickEntity;
            modeStatus->SetText("Pick");
            break;
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
            break;
        default:
            assert(false);
    }
}

RayCastInfo Editor::castRayScreen(bool fromCenter)
{
    sf::Vector2u sz = p_input->mainWindow->getSize();
    int screenWidth = sz.x - leftWndWidth - rghtWndWidth,
        screenHeight = sz.y - topWndHeight - dwnWndHeight;
    sf::Vector2i mp(screenWidth / 2, screenHeight / 2);
    if (!fromCenter) {
        mp = sf::Mouse::getPosition(*p_input->mainWindow);
        mp.x -= leftWndWidth + rghtWndWidth;
        mp.y -= topWndHeight + dwnWndHeight;
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

void Editor::update()
{
    p_input->update();
    auto &physics = p_world->physics;
    auto &graphics = *p_world->pGraphics;
    Camera *cam = graphics.getCamera();

    physics.render(graphics.getCamera());
    cam->moveFree(vCameraSpeed * editorFlySpeed * egg::getInstance().g_Delta);

    if (p_input->lockMouse) {
        cam->rotate(0.1f * egg::getInstance().g_Delta * p_input->mouseDelta.x,
                    0.1f * egg::getInstance().g_Delta * p_input->mouseDelta.y);
    } else {
        if (p_input->cursorIsInsideWindow()) {
            if (edMode == Pulling) {
                if (!selectedEntity) {
                    changeMode(Idle);
                    return;
                }
                sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow);
                glm::vec3 camPos = graphics.getCamera()->getPosition();
                const glm::vec3& enPos = selectedEntity->getProperty<glm::vec3>("Position");
                camPos = camPos - enPos;
                sf::Vector2u sz = p_input->mainWindow->getSize();
                camPos *= editorFlySpeed * ((mpos.y - mposOffsetMoving.y) / (1.0f * sz.y)) / glm::length(camPos);
                selectedEntity->setProperty("Position", enPos + camPos);
                mposOffsetMoving = glm::vec2(mpos.x, mpos.y);

            } else if (edMode == Moving) {
                if (!selectedEntity) {
                    changeMode(Idle);
                    return;
                }
                glm::mat4 camMat = graphics.getCamera()->getProjectionMatrix() * graphics.getCamera()->getViewMatrix();
                const glm::vec3& enPosV3 = selectedEntity->getProperty<glm::vec3>("Position");
                glm::vec4 entPos = glm::vec4(enPosV3.x, enPosV3.y, enPosV3.z, 1.0f);
                entPos = camMat * entPos;
                sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow) - sf::Vector2i(leftWndWidth + rghtWndWidth, topWndHeight + dwnWndHeight);
                sf::Vector2u sz = p_input->mainWindow->getSize();
                mpos.y = sz.y - mpos.y;
                int screenWidth = sz.x - leftWndWidth - rghtWndWidth,
                    screenHeight = sz.y - topWndHeight - dwnWndHeight;
                glm::vec4 endPosNDC = glm::vec4((float(mpos.x) / float(screenWidth)) * 2.0f - 1.0f + mposOffsetMoving.x,
                                                (float(mpos.y) / float(screenHeight)) * 2.0f - 1.0f + mposOffsetMoving.y,
                                                entPos.z / entPos.w,
                                                1.0f);
                glm::vec4 endPosWLD = glm::inverse(camMat) * endPosNDC;
                endPosWLD /= endPosWLD.w;
                selectedEntity->setProperty("Position", glm::vec3(endPosWLD.x, endPosWLD.y, endPosWLD.z));
            }
        }
    }
}

void Editor::drawGrid() const
{
    glm::vec3 camPos = p_world->pGraphics->getCamera()->getPosition();

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
        glVertex3f(camPos.x + 100.0f, 0.0f, 0.0f);
        glVertex3f(camPos.x - 100.0f, 0.0f, 0.0f);

        glVertex3f(0.0f, 0.0f, camPos.z + 100.0f);
        glVertex3f(0.0f, 0.0f, camPos.z - 100.0f);
    glEnd();

    int baseX = (int)camPos.x;
    int baseZ = (int)camPos.z;

    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_LINES);
        for(int i=-100; i<100; ++i)
        {
            glVertex3f(baseX+i, 0.0f, camPos.z + 100.0f);
            glVertex3f(baseX+i, 0.0f, camPos.z - 100.0f);
        }
        for(int i=-100; i<100; ++i)
        {
            glVertex3f(camPos.x + 100.0f, 0.0f, baseZ+i);
            glVertex3f(camPos.x - 100.0f, 0.0f, baseZ+i);
        }
    glEnd();

    glClear(GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-2.0f, 2.0f, -1.5f, 1.5f, 0.1f, 100.0f);

    const glm::mat4& rotMx = p_world->pGraphics->getCamera()->getBcgMatrix();

    glm::vec4 vecX(0.1f, 0.0f, 0.0f, 1.0f);
    glm::vec4 vecY(0.0f, 0.1f, 0.0f, 1.0f);
    glm::vec4 vecZ(0.0f, 0.0f, 0.1f, 1.0f);
    vecX = rotMx * vecX;
    vecY = rotMx * vecY;
    vecZ = rotMx * vecZ;

    glTranslatef(-1.9f, -1.4f, -20.0f);

    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(vecX.x, vecX.y, vecX.z);
    glEnd();
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(vecY.x, vecY.y, vecY.z);
    glEnd();
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(vecZ.x, vecZ.y, vecZ.z);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glColor3f(1.0f, 1.0f, 1.0f);
}
