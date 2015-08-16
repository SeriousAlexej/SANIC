#include <string>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <AntTweakBar.h>
#include <boost/filesystem.hpp>
#include <rapidjson/writer.h>

#include "editor.h"
#include "world.h"
#include "./entities/player.h"
#include "./entities/box.h"
#include "./entities/decoration.h"
#include "./entities/pointlight.h"

sf::Clock g_Clock;
float	  g_LastTime = 0.0f;
float	  g_Delta = 0.0f;
bool	  g_Editor = false;
sf::Vector2u g_Resolution(1024u, 768u);
bool g_UseDirectionalLight = true;
std::string g_WorkingDir;

const static char logo[] = {
"              _^  ^\n            _/  \\/ |\n        ___/ _/\\_\\/|\n     __/          \\|\n   _/         ====\\\\\n _/   _     //     |\n/____/     ||  ()  |\n  _/   __  ||      |\n /   _/   __\\\\____/\\()\n/___/    /__       /\n  _/     \\ \\__    /\n  /________\\_____/\n   _____ ___    _   ____________\n  / ___//   |  / | / /  _/ ____/\n  \\__ \\/ /| | /  |/ // // /     \n ___/ / ___ |/ /|  // // /___   \n/____/_/  |_/_/ |_/___/\\____/   \n"};

void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString)
{
  // Copy the content of souceString handled by the AntTweakBar library to destinationClientString handled by your application
  destinationClientString = sourceLibraryString;
}

/*int main(int argc, char **argv) // TODO: make an external launcher instead
{
    g_WorkingDir = boost::filesystem::current_path().string();
    std::replace(g_WorkingDir.begin(), g_WorkingDir.end(), '\\', '/');
	g_Editor = (argc > 1);
	printf(logo);
	sf::sleep(sf::seconds(1.0f));

	sf::ContextSettings cs;
	cs.antialiasingLevel = 4;
	cs.depthBits = 24;
	cs.majorVersion = 3;
	cs.minorVersion = 3;
	sf::RenderWindow window(sf::VideoMode(g_Resolution.x, g_Resolution.y), "SANIC", sf::Style::Default & ~sf::Style::Resize, cs);
    window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);

    // Make it the active window for OpenGL calls
    window.setActive();

	// Initialize GLEW
	glewExperimental=true;
	if(glewInit() != GLEW_OK)
	{
		fprintf( stderr, "Failed to initialize GLEW\n" );
		return -1;
	}
	TwInit(TW_OPENGL, NULL);
	TwWindowSize(window.getSize().x,window.getSize().y);
	TwCopyStdStringToClientFunc(CopyStdStringToClient);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    World* wld = new World();
	World &world = *wld;

    world.createEntity("Decoration");
    world.createEntity("Decoration");
    world.createEntity("PointLight");
    world.createEntity("DirectionalLight");
	
    Editor* editor = new Editor(&window, wld);

	g_Clock.restart();
    // Start game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
			int handled = TwEventSFML23(&event);
			if(!handled)
			{
                editor->getInputHandler()->allowCheck = true;

				switch(event.type)
				{
				case sf::Event::LostFocus:
                    {
                        editor->getInputHandler()->setFocus(false);
                        break;
                    }
                case sf::Event::GainedFocus:
                    {
                        editor->getInputHandler()->setFocus(true);
                        break;
                    }
				case sf::Event::Closed:
					{
						window.close();
						break;
					}
				case sf::Event::MouseWheelMoved:
					{
                        editor->getInputHandler()->registerWheelDelta(event.mouseWheel.delta);
						break;
					}
				default: break;
				}
			}
			else //if TwEventSFML handled
			{
                editor->getInputHandler()->allowCheck = false;
			}
        }

		float currTime = g_Clock.getElapsedTime().asSeconds();
		g_Delta = currTime - g_LastTime;
		g_LastTime = currTime;
		
        if(editor->getInputHandler()->isFocused())
        {
            editor->update();
            world.update();
            TwDraw();
		}

        window.display();
	}

	TwTerminate();

	delete wld;

	return EXIT_SUCCESS;
}
*/
