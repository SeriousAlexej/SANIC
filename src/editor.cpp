#include <string>
#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>
#include <boost/filesystem.hpp>
#include <AntTweakBar.h>
#include "dialogs/tinyfiledialogs.h"
#include "world.h"
#include "input_handler.h"
#include "editor.h"
#include "global.h"
#include "editorGUI.h"

class IncuButton : public sfg::Button
{
public:
    typedef std::shared_ptr<IncuButton> Ptr;
    static Ptr Create(const sf::String &lbl, World* w)
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
    IncuButton(World* w) : wld(w) {}
    World* wld;
};


void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString)
{
  destinationClientString = sourceLibraryString;
}

static std::string relativePath(std::string absPath)
{
    std::replace(absPath.begin(), absPath.end(), '\\', '/');
    if(absPath.find(g_WorkingDir)==0)
    {
        absPath = "." + absPath.substr(g_WorkingDir.length());
    }
    if(absPath[0] != '.')
    {
        absPath = "." + absPath;
    }
    return absPath;
}

void Editor::NewWorld()
{
    if(selectedEntity != nullptr)
    {
        selectedEntity->editorDesselect();
        selectedEntity = nullptr;
    }
    p_world->Clear();
}

void Editor::Load()
{
    std::string path;
    const char * result = tinyfd_openFileDialog("Load world","./",0,NULL,0);
    if(result) {
        path = relativePath(result);
        p_world->Love(path);
    }
}

void Editor::SaveAs()
{
    std::string path;
    const char * result = tinyfd_saveFileDialog("Save world as","./",0,NULL);
    if(result) {
        path = relativePath(result);
        p_world->Save(path);
    }
}

void Editor::resizeGUIComponents(unsigned width, unsigned height)
{
	TwWindowSize(width, height);

    topWindow->SetAllocation( sf::FloatRect( 0.0f, 0.0f,
                                       static_cast<float>(width), static_cast<float>(topWndHeight) ) );
    leftWindow->SetAllocation( sf::FloatRect( 0.0f, static_cast<float>(topWndHeight),
                                       static_cast<float>(leftWndWidth), static_cast<float>(height - topWndHeight)));

    g_DrawOrigin.x = leftWndWidth;
    g_DrawOrigin.y = 0u;

    g_Resolution.x = width - leftWndWidth;
    g_Resolution.y = height - topWndHeight;

    if(selectedEntity != nullptr)
    {
        std::string barSize = "size='"+std::to_string(barWidth)+" "+std::to_string(g_Resolution.y)+"' ";
        TwDefine((" EntityBar " + barSize).c_str());
    }
}

int Editor::run()
{
	printf(logo);

	sf::ContextSettings cs;
	cs.antialiasingLevel = 4;
	cs.depthBits = 24;
	cs.majorVersion = 3;
	cs.minorVersion = 3;

	sf::RenderWindow window(sf::VideoMode(g_Resolution.x, g_Resolution.y), "Eggine Editor", sf::Style::Default, cs);
    window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);
    window.setActive();

	glewExperimental=true;
	glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

	TwInit(TW_OPENGL, NULL);
	TwCopyStdStringToClientFunc(CopyStdStringToClient);

	p_input = new InputHandler(&window);
	p_world = new World();

	sfg::SFGUI sfgui;
    window.setActive();
    sfg::Desktop desktop;
    leftWindow = sfg::Window::Create(sfg::Window::Style::BACKGROUND);
    topWindow = sfg::Window::Create(sfg::Window::Style::BACKGROUND);
    //fill windows here
    {
        auto leftBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 2.0f);
        std::vector<std::string> classes = Incubator::getRegisteredClasses();
        for(std::string &cl : classes)
        {
            auto btn = IncuButton::Create(cl, p_world);
            btn->GetSignal( sfg::Widget::OnLeftClick ).Connect( std::bind( &IncuButton::Spawn, btn ) );
            leftBox->Pack(btn, true);
        }
        auto leftScroll = sfg::ScrolledWindow::Create();
        leftScroll->AddWithViewport(leftBox);
        auto mainLeftBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 0.0f);
        auto lbl = sfg::Label::Create("Create Entity");
        mainLeftBox->Pack(lbl, false, true);
        mainLeftBox->Pack(leftScroll);
        leftWindow->Add(mainLeftBox);

        auto topBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 2.0f);
        auto btnNew = sfg::Button::Create("New World");
        auto btnLoad = sfg::Button::Create("Load World");
        auto btnSaveAs = sfg::Button::Create("Save As");
        auto btnSave = sfg::Button::Create("Save");
        btnNew->SetRequisition(sf::Vector2f(100.0f, 0.0f));
        btnLoad->SetRequisition(sf::Vector2f(100.0f, 0.0f));
        btnSaveAs->SetRequisition(sf::Vector2f(100.0f, 0.0f));
        btnSave->SetRequisition(sf::Vector2f(100.0f, 0.0f));
        btnNew->GetSignal( sfg::Widget::OnLeftClick ).Connect( std::bind( &Editor::NewWorld, this) );
        btnLoad->GetSignal( sfg::Widget::OnLeftClick ).Connect( std::bind( &Editor::Load, this) );
        btnSaveAs->GetSignal( sfg::Widget::OnLeftClick ).Connect( std::bind( &Editor::SaveAs, this) );
        //TODO: bind Save button
        topBox->Pack(btnNew, false, true);
        topBox->Pack(btnLoad, false, true);
        topBox->Pack(btnSaveAs, false, true);
        topBox->Pack(btnSave, false, true);
        topWindow->Add(topBox);
    }
    //~fill windows here
    desktop.Add(leftWindow);
    desktop.Add(topWindow);
    resizeGUIComponents(window.getSize().x, window.getSize().y);
    desktop.Update( 0.f );

	g_Clock.restart();
    while (window.isOpen())
    {
		float currTime = g_Clock.getElapsedTime().asSeconds();
		g_Delta = currTime - g_LastTime;
		g_LastTime = currTime;

		desktop.Update(g_Delta);

        sf::Event event;
        while (window.pollEvent(event))
        {
            int handled = TwEventSFML23(&event);
            if(!handled)
            {
                p_input->allowCheck = true;

                switch(event.type)
                {
                case sf::Event::LostFocus:
                    {
                        p_input->setFocus(false);
                        break;
                    }
                case sf::Event::GainedFocus:
                    {
                        p_input->setFocus(true);
                        break;
                    }
                case sf::Event::Closed:
                    {
                        window.close();
                        break;
                    }
                case sf::Event::MouseWheelMoved:
                    {
                        p_input->registerWheelDelta(event.mouseWheel.delta);
                        break;
                    }
                case sf::Event::Resized:
                    {
                        unsigned width(event.size.width), height(event.size.height);
                        bool fixSize = false;
                        if(width < 320)
                        {
                            fixSize = true;
                            width = 320;
                        }
                        if(height < 240)
                        {
                            fixSize = true;
                            height = 240;
                        }
                        if(fixSize)
                        {
                            window.setSize(sf::Vector2u(width, height));
                        }
                        resizeGUIComponents(width, height);
                        break;
                    }
                default: break;
                }
            }
            else //if TwEventSFML handled
            {
                p_input->allowCheck = false;
            }
            if(!p_input->cursorIsInsideWindow())
            {
                desktop.HandleEvent(event);
            }
        }

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        if(p_input->isFocused())
        {
            update();
            p_world->update();
            TwDraw();
		}
		glPopAttrib();

        window.pushGLStates();
        window.resetGLStates();
        glViewport(0, 0, window.getSize().x, window.getSize().y);
		sfgui.Display(window);
		window.popGLStates();

        window.display();
	}

	TwTerminate();

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
    g_WorkingDir = boost::filesystem::current_path().string();
    std::replace(g_WorkingDir.begin(), g_WorkingDir.end(), '\\', '/');
	g_Editor = true;

    Editor::Ptr ed(new Editor());
    return ed;
}

RayCastInfo Editor::castRayScreen(bool fromCenter)
{
    sf::Vector2u sz = p_input->mainWindow->getSize();
	int screenWidth = sz.x - leftWndWidth, screenHeight = sz.y - topWndHeight;
	sf::Vector2i mp(screenWidth/2, screenHeight/2);
	if(!fromCenter)
	{
		mp = sf::Mouse::getPosition(*p_input->mainWindow);
		mp.x -= leftWndWidth; mp.y -= topWndHeight;
		mp.y = screenHeight - mp.y;
    }

	int mouseX = mp.x, mouseY = mp.y;

	glm::vec4 lRayStart_NDC(
		(float(mouseX)/float(screenWidth)  - 0.5f) * 2.0f,
		(float(mouseY)/float(screenHeight) - 0.5f) * 2.0f,
		-1.0,
		1.0f
		);
	glm::vec4 lRayEnd_NDC(
		(float(mouseX)/float(screenWidth)  - 0.5f) * 2.0f,
		(float(mouseY)/float(screenHeight) - 0.5f) * 2.0f,
		1.0,
		1.0f
		);

    WorldGraphics& graphics = *p_world->pGraphics;

    glm::mat4 M = glm::inverse(graphics.getCamera()->getProjectionMatrix() * graphics.getCamera()->getViewMatrix());
	glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
	glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;

    glm::vec4 lrd4(lRayEnd_world - lRayStart_world);
	glm::vec3 lRayDir_world = glm::vec3(lrd4.x, lrd4.y, lrd4.z);
	glm::vec3 lRayOrigin_world = glm::vec3(lRayStart_world.x, lRayStart_world.y, lRayStart_world.z);

	return p_world->castRay(lRayOrigin_world, lRayDir_world);
}

void Editor::updateEntity(Entity *pen)
{

}

void Editor::update()
{
	p_input->update();
    auto& physics = p_world->physics;
    auto& graphics = *p_world->pGraphics;

    graphics.render(); //DO NOT CHANGE RENDERING ORDER, IT'S ADDITIVE :P
    physics.render(graphics.getCamera());

	if(selectedEntity != nullptr)
	{
		selectedEntity->editorUpdate();
	}
    static float doubleClickTime = g_Clock.getElapsedTime().asSeconds();
    static float moveModeTime = doubleClickTime;
    static float moveModePeriod = 1.0f;
    float tmNow = g_Clock.getElapsedTime().asSeconds();
	if(p_input->lockMouse)
	{
        edMode = Fly;
		if(p_input->wheelDelta > 0)
		{
			p_input->wheelDelta = 0;
			if(editorFlySpeed < 100.0f)
				editorFlySpeed *= 2.0f;
		} else
		if(p_input->wheelDelta < 0)
		{
			p_input->wheelDelta = 0;
			if(editorFlySpeed > 0.25f)
				editorFlySpeed *= 0.5f;
		}
		Camera* cam = graphics.getCamera();
		cam->rotate(0.1f * g_Delta * p_input->mouseDelta.x,
					0.1f * g_Delta * p_input->mouseDelta.y);
        if(p_input->keyPressed(sf::Keyboard::Up))
		{
			cam->moveFront(editorFlySpeed*g_Delta);
		} else
        if(p_input->keyPressed(sf::Keyboard::Down))
		{
			cam->moveFront(-editorFlySpeed*g_Delta);
		}
        if(p_input->keyPressed(sf::Keyboard::Right))
		{
			cam->moveRight(editorFlySpeed*g_Delta);
		} else
        if(p_input->keyPressed(sf::Keyboard::Left))
		{
			cam->moveRight(-editorFlySpeed*g_Delta);
		}
		if(p_input->keyPressed(sf::Keyboard::Space))
		{
			cam->moveUp(editorFlySpeed*g_Delta);
		} else
		if(p_input->keyPressed(sf::Keyboard::C))
		{
			cam->moveUp(-editorFlySpeed*g_Delta);
		}
	} else {
        if(edMode == Fly)
        {
            edMode = Idle;
        }
		if(p_input->cursorIsInsideWindow())
		{

        if(edMode==Idle && p_input->keyJustReleased(sf::Keyboard::Delete))
        {
            if(selectedEntity != NULL)
            {
                selectedEntity->editorDesselect();
                p_world->removeEntity(selectedEntity);
                selectedEntity = NULL;
                return;
            }
        }

        if(edMode==Idle && p_input->keyJustReleased(sf::Keyboard::Q))
        {
            if(selectedEntity != NULL)
            {
                selectedEntity->editorDesselect();
                selectedEntity = NULL;
                return;
            }
        }

        if(selectedEntity != NULL && edMode == Idle && p_input->keyJustReleased(sf::Keyboard::R))
        {
            RayCastInfo ri = castRayScreen();
            EntityPointer* ep = selectedEntity->getTargetPointer();
            if(ep != nullptr)
            {
                if(ep->Name() == "Parent")
                {
                    selectedEntity->setParent(ri.enHit);
                } else {
                    (*ep) = ri.enHit;
                }
                selectedEntity->pointerIndexPrevious = -1; //update target info string
            }
        }

		if(selectedEntity != NULL && edMode == Idle && p_input->keyPressed(sf::Keyboard::LControl) && p_input->mouseButtonJustPressed(sf::Mouse::Right))
		{
		    edMode = Pulling;
            sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow);
            mposOffsetMoving = glm::vec2(mpos.x, mpos.y);
		}

		if(edMode == Pulling)
        {
            if(selectedEntity == NULL) { edMode = Idle; return; }
            sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow);
            glm::vec3 camPos = graphics.getCamera()->getPosition();
            camPos = camPos - selectedEntity->position;
            camPos *= editorFlySpeed*((mpos.y - mposOffsetMoving.y)/(1.0f*p_input->windowSize.y))/glm::length(camPos);
            selectedEntity->position += camPos;
            mposOffsetMoving = glm::vec2(mpos.x, mpos.y);
        }

		if(selectedEntity != NULL && edMode == Idle && p_input->keyPressed(sf::Keyboard::LControl) && p_input->mouseButtonJustPressed(sf::Mouse::Left))
		{
		    if(tmNow-doubleClickTime<=0.5f && (moveModePeriod<0.1f || tmNow-moveModeTime-moveModePeriod>=1.0f))
            {
                RayCastInfo ri = castRayScreen();
                if(ri.enHit!=NULL)
                {
                    selectedEntity->position = ri.posHit;
                } else {
                    selectedEntity->position = ri.posOrigin+ri.direction*5.0f;
                }
                doubleClickTime -= 10.0f;
                return;
            }
		    doubleClickTime = tmNow;

            edMode = Moving;
            moveModeTime = tmNow;
            sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow);
            mpos.y = p_input->windowSize.y - mpos.y;
            glm::mat4 camMat = graphics.getCamera()->getProjectionMatrix() * graphics.getCamera()->getViewMatrix();
            glm::vec4 entPos = glm::vec4(selectedEntity->position.x, selectedEntity->position.y, selectedEntity->position.z, 1.0f);
            entPos =  camMat * entPos; entPos/=entPos.w;
            mposOffsetMoving = glm::vec2(entPos.x - (float(mpos.x)/float(p_input->windowSize.x))*2.0f + 1.0f,
                                         entPos.y - (float(mpos.y)/float(p_input->windowSize.y))*2.0f + 1.0f);
		}

		if(edMode == Moving)
		{
            if(selectedEntity == NULL) { edMode = Idle; return; }
            glm::mat4 camMat = graphics.getCamera()->getProjectionMatrix() * graphics.getCamera()->getViewMatrix();
            glm::vec4 entPos = glm::vec4(selectedEntity->position.x, selectedEntity->position.y, selectedEntity->position.z, 1.0f);
            entPos =  camMat * entPos;
            sf::Vector2i mpos = sf::Mouse::getPosition(*p_input->mainWindow);
            mpos.y = p_input->windowSize.y - mpos.y;
            glm::vec4 endPosNDC = glm::vec4( (float(mpos.x)/float(p_input->windowSize.x))*2.0f - 1.0f + mposOffsetMoving.x,
                                             (float(mpos.y)/float(p_input->windowSize.y))*2.0f - 1.0f + mposOffsetMoving.y,
                                             entPos.z/entPos.w,
                                             1.0f);
            glm::vec4 endPosWLD = glm::inverse(camMat) * endPosNDC; endPosWLD/=endPosWLD.w;
            selectedEntity->position = glm::vec3(endPosWLD.x, endPosWLD.y, endPosWLD.z);
		}


		if(p_input->mouseButtonJustReleased(sf::Mouse::Right))
		{
            if(edMode != Idle) { edMode = Idle; return; }
		}

		if(p_input->mouseButtonJustReleased(sf::Mouse::Left))
		{
            if(edMode != Idle)
            {
                if(edMode == Moving)
                {
                    moveModePeriod = tmNow - moveModeTime;
                }
                edMode = Idle;
                return;
            }

            if(tmNow-doubleClickTime<0.5f) return;
            RayCastInfo rci = castRayScreen();
            if(rci.enHit != NULL)
            {
                if(selectedEntity != nullptr)
                {
                    if(selectedEntity == rci.enHit) return;
                    selectedEntity->editorDesselect();
                }
                selectedEntity = rci.enHit;
                selectedEntity->editorSelect();
            } else {
                if(selectedEntity != nullptr)
                {
                    selectedEntity->editorDesselect();
                    selectedEntity = nullptr;
                }
            }
		}
		}
	}
}
