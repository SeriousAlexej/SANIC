#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>
#include "world.h"
#include "input_handler.h"
#include "global.h"
#include "game.h"

Game::Ptr Game::Create()
{
    Game::Ptr gm(new Game());
    return gm;
}

Game::Game()
{
}

Game::~Game()
{
    delete window;
    delete p_input;
    delete p_world;
}

void Game::setup()
{
    sf::ContextSettings cs;
    cs.antialiasingLevel = 4;
    cs.depthBits = 24;
    cs.majorVersion = 3;
    cs.minorVersion = 0;
    cs.stencilBits = 8;

    egg::getInstance().g_Resolution = sf::Vector2u(1024u, 768u);
    egg::getInstance().g_DrawOrigin = sf::Vector2u(0u, 0u);

    window = new sf::RenderWindow(sf::VideoMode(egg::getInstance().g_Resolution.x, egg::getInstance().g_Resolution.y), "Sanic", sf::Style::Titlebar | sf::Style::Close, cs);
    window->setVerticalSyncEnabled(true);
    //window->setFramerateLimit(60);
    window->setActive();

    glewExperimental=true;
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    p_input = new InputHandler(window);
    p_world = new World();
    egg::getInstance().g_World = p_world;
}

int Game::run()
{
    printf("%s", egg::getInstance().logo.c_str());

    if(!startupWorld.empty())
    {
        p_world->Love(startupWorld);
    }
    //p_world->createEntity("Player");

    float sec = 0.0f;
    unsigned fps = 0u;

    egg::getInstance().g_Clock.restart();
    while (window->isOpen())
    {
        float currTime = egg::getInstance().g_Clock.getElapsedTime().asSeconds();
        egg::getInstance().g_Delta = currTime - egg::getInstance().g_LastTime;
        egg::getInstance().g_LastTime = currTime;

        sf::Event event;
        while (window->pollEvent(event))
        {
            int handled = false; //event was handled by gui
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
                        window->close();
                        break;
                    }
                case sf::Event::MouseWheelMoved:
                    {
                        p_input->registerWheelDelta(event.mouseWheel.delta);
                        break;
                    }
                default: break;
                }
            }
            else //if gui event handled
            {
                p_input->allowCheck = false;
            }
        }
	    p_input->update();

        if(p_input->isFocused())
        {

            float editorFlySpeed = 3.0f;

		Camera* cam = p_world->pGraphics->getCamera();
        cam->rotate(0.1f * egg::getInstance().g_Delta * p_input->mouseDelta.x,
                    0.1f * egg::getInstance().g_Delta * p_input->mouseDelta.y);
        if(p_input->keyPressed(sf::Keyboard::Up) || p_input->keyPressed(sf::Keyboard::W))
		{
            cam->moveFront(editorFlySpeed*egg::getInstance().g_Delta);
		} else
        if(p_input->keyPressed(sf::Keyboard::Down) || p_input->keyPressed(sf::Keyboard::S))
		{
            cam->moveFront(-editorFlySpeed*egg::getInstance().g_Delta);
		}
        if(p_input->keyPressed(sf::Keyboard::Right) || p_input->keyPressed(sf::Keyboard::D))
		{
            cam->moveRight(editorFlySpeed*egg::getInstance().g_Delta);
		} else
        if(p_input->keyPressed(sf::Keyboard::Left) || p_input->keyPressed(sf::Keyboard::A))
		{
            cam->moveRight(-editorFlySpeed*egg::getInstance().g_Delta);
		}
		if(p_input->keyPressed(sf::Keyboard::Space))
		{
            cam->moveUp(editorFlySpeed*egg::getInstance().g_Delta);
		} else
		if(p_input->keyPressed(sf::Keyboard::C))
		{
            cam->moveUp(-editorFlySpeed*egg::getInstance().g_Delta);
		}
            //glClear(GL_COLOR_BUFFER_BIT);
            p_world->update();
            //p_world->physics.render(cam);
		}

        window->display();

        ++fps;
        sec += egg::getInstance().g_Delta;
        if(sec >= 1.0f) {
            sec = 0.0f;
            printf("%u\n", fps);
            fps = 0u;
        }
	}

    return EXIT_SUCCESS;
}
