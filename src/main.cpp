#include <string>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <AntTweakBar.h>
#include <boost/filesystem.hpp>

#include "world.h"
#include "./entities/player.h"
#include "./entities/box.h"
#include "./entities/decoration.h"
#include "./entities/pointlight.h"

sf::Clock g_Clock;
float	  g_LastTime = 0.0f;
float	  g_Delta = 0.0f;
bool	  g_Editor = false;
std::string g_WorkingDir;

const static char logo[] = {
"              _^  ^\n            _/  \\/ |\n        ___/ _/\\_\\/|\n     __/          \\|\n   _/         ====\\\\\n _/   _     //     |\n/____/     ||  ()  |\n  _/   __  ||      |\n /   _/   __\\\\____/\\()\n/___/    /__       /\n  _/     \\ \\__    /\n  /________\\_____/\n   _____ ___    _   ____________\n  / ___//   |  / | / /  _/ ____/\n  \\__ \\/ /| | /  |/ // // /     \n ___/ / ___ |/ /|  // // /___   \n/____/_/  |_/_/ |_/___/\\____/   \n"};

void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString)
{
  // Copy the content of souceString handled by the AntTweakBar library to destinationClientString handled by your application
  destinationClientString = sourceLibraryString;
}

int main(int argc, char **argv)
{
    g_WorkingDir = boost::filesystem::current_path().string();
    std::replace(g_WorkingDir.begin(), g_WorkingDir.end(), '\\', '/');
	g_Editor = (argc > 1);
	printf(logo);
	sf::sleep(sf::seconds(1.0f));

	sf::ContextSettings cs;
	cs.antialiasingLevel = 4;
	cs.depthBits = 32;
	cs.majorVersion = 2;
	cs.minorVersion = 1;
	sf::RenderWindow window(sf::VideoMode(1024, 768), "SANIC", sf::Style::Default & ~sf::Style::Resize, cs);
    window.setVerticalSyncEnabled(true);
	//window.setFramerateLimit(60);

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
	glDepthFunc(GL_LESS);
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    World* wld = new World(&window);
	World &world = *wld;
	//Player* e = (Player*)world.createEntity((Entity*)(new Player()));
	//SolidBody* floor = world.physics.addBody(0, glm::vec3(20,1,20));
	ModelInstance* M = (ModelInstance*)world.graphics.createModel("./shaders/smooth.shader",
											 "./models/sanic.obj",
											 "./models/sanic.tga", "", "");
    M->playAnimation("walk");
    //Box* b1 = (Box*)world.createEntity((Entity*) new Box());
    //Box* b2 = (Box*)world.createEntity((Entity*) new Box());
    //Box* b3 = (Box*)world.createEntity((Entity*) new Box());
    Decoration* dec1 = (Decoration*)world.createEntity((Entity*) new Decoration());
    PointLight* pl1 = (PointLight*)world.createEntity((Entity*) new PointLight());
    PointLight* pl2 = (PointLight*)world.createEntity((Entity*) new PointLight());
    PointLight* pl3 = (PointLight*)world.createEntity((Entity*) new PointLight());
    PointLight* pl4 = (PointLight*)world.createEntity((Entity*) new PointLight());

	//test
	//e->setupModel("./shaders/smooth.vsh", "./shaders/smooth.fsh",
	//										 "./models/sanic.obj",
	//										 "./models/sanic.tga", "./models/normal.jpg");

//	WorldGraphics worldGFX;
//	WorldPhysics worldPHY;

	//ModelInstance* mi;
	/* = worldGFX.createModel("./shaders/smooth.vsh", "./shaders/smooth.fsh",
											 "./models/test/test.obj",
											 "./models/test/test_CM.png", "./models/test/test_NM.png");
	mi->setPosition(glm::vec3(2,0,0));
	mi = worldGFX.createModel("./shaders/normal.vsh", "./shaders/normal.fsh",
											 "./models/test/test.obj",
											 "./models/test/test_CM.png", "./models/test/test_NM.png");
	mi->setPosition(glm::vec3(0,0,2));
	*/
	/*
	mi = worldGFX.createModel("./shaders/smooth.vsh", "./shaders/smooth.fsh",
											 "./models/sanic.obj",
											 "./models/sanic.tga", "./models/normal.jpg");
	mi->setPosition(glm::vec3(-2,0,0));
	mi = worldGFX.createModel("./shaders/normal.vsh", "./shaders/normal.fsh",
											 "./models/sanic.obj",
											 "./models/sanic.tga", "./models/normal.jpg");
	mi->playAnimation("walk");
	mi->setPosition(glm::vec3(0,0,-2));
	SolidBody* floor = worldPHY.addBody(0, glm::vec3(20,1,20));
	floor->setPosition(glm::vec3(0,-1,0));
	SolidBody* cube = worldPHY.addBody(0, glm::vec3(3, 1, 2));
	//cube->setPosition(glm::vec3(0,1,5));
	SolidBody* sphere = worldPHY.addBody(20, 1.0f);

	ModelInstance* axis = worldGFX.createModel("./shaders/fullbright.vsh", "./shaders/fullbright.fsh",
											 "",
											 "./models/uv_checker.jpg", "");

	Light* light = worldGFX.createLight();
	light->setPosition(glm::vec3(0,3,-2));
	light->setDiffuseColor(glm::vec3(1,1,1));
	light->setAmbientColor(glm::vec3(0.3,0.3,0.3));
	light->setFallOff(5.0f);
	light->setHotSpot(4.0f);

	light = worldGFX.createLight();
	light->setPosition(glm::vec3(-2,2,-2));
	light->setDiffuseColor(glm::vec3(1,0.75,0.75));
	light->setAmbientColor(glm::vec3(0.5,0.1,0.1));
	light->setFallOff(4.0f);
	light->setHotSpot(3.0f);


	Camera* cm = worldGFX.getCamera();
	cm->setOffset(glm::vec3(0,0,3));
	cm->setRotation(3.14f,0.0f);
	cm->setPosition(glm::vec3(0,1,0));
*/
	//TwBar *myBar;
	//myBar = TwNewBar("NameOfMyTweakBar");

	g_Clock.restart();
	/*
	double lastTime = g_Clock.getElapsedTime().asSeconds();
    int nbFrames = 0;
    */
    // Start game loop
    while (window.isOpen())
    {
        /*
        double currentTime = g_Clock.getElapsedTime().asSeconds();
         nbFrames++;
         if ( currentTime - lastTime >= 1.0 ){
             printf("%f ms/frame\n", 1000.0/double(nbFrames));
             nbFrames = 0;
             lastTime += 1.0;
         }
         */
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
			int handled = TwEventSFML(&event, 2, 2);
			if(!handled)
			{
				world.getInputHandler()->allowCheck = true;

				switch(event.type)
				{
				case sf::Event::LostFocus:
                    {
                        world.getInputHandler()->setFocus(false);
                        break;
                    }
                case sf::Event::GainedFocus:
                    {
                        world.getInputHandler()->setFocus(true);
                        break;
                    }
				case sf::Event::Closed:
					{
						window.close();
						break;
					}
				case sf::Event::MouseWheelMoved:
					{
						world.getInputHandler()->registerWheelDelta(event.mouseWheel.delta);
						break;
					}
				default: break;
				}
			}
			else //if TwEventSFML handled
			{
				world.getInputHandler()->allowCheck = false;
			}
        }

		float currTime = g_Clock.getElapsedTime().asSeconds();
		g_Delta = currTime - g_LastTime;
		g_LastTime = currTime;

		//sf::Vector2i mpos = sf::Mouse::getPosition(window);

		//cm->rotate(mouseSpeed * g_Delta * float(1024/2 - mpos.x ),
		//	       mouseSpeed * g_Delta * float( 768/2 - mpos.y ));

/*
		glm::vec3 newpos(0,0,0);
		// Move forward
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
			newpos.x += (speed);
		} else
		// Move backward
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
			newpos.x += (-speed);
		}
		// Strafe right
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
			newpos.z += (speed);
		} else
		// Strafe left
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
			newpos.z += (-speed);
		}
*/
		//e->setDesiredADir(newpos);

/*
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
			cube->rotate(glm::vec3(3.14*g_Delta,0,0));
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
			cube->rotate(glm::vec3(-3.14*g_Delta,0,0));
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)){
			cube->translate(glm::vec3(0,-1*g_Delta,0));//setVelocity(glm::vec3(0,-1,0));
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)){
			cube->translate(glm::vec3(0,1*g_Delta,0));//setVelocity(glm::vec3(0,1,0));
		}

		newpos = newpos.z * cm->getFront() + newpos.x * cm->getRight();

		//shitty test code
		float avel = glm::length(sphere->getAngularVelocity());
		if(avel > 0.5f)
		{
			glm::vec3 vel = sphere->getVelocity();
			glm::vec2 vel2d(vel.x, -vel.z);
			float h = 0.0f;
			vel2d = glm::normalize(vel2d);
			if(vel2d.x > 0 && vel2d.y > 0)
				h = acos(vel2d.x);
			else
			if(vel2d.x > 0 && vel2d.y < 0)
				h = -acos(vel2d.x);
			else
			if(vel2d.x < 0 && vel2d.y > 0)
				h = 1.57 + acos(vel2d.y);
			else
			if(vel2d.x < 0 && vel2d.y < 0)
				h = -1.57 - acos(-vel2d.y);

			mi->setRotation(glm::vec3(0,-90.0f+h*57.3f,0));
		}
		sphere->setAngularVelocity(newpos*100.0f);
		mi->setPosition(sphere->getPosition() + glm::vec3(0,-1,0));
		cm->setPosition(sphere->getPosition());
*/
		//mi->translate(newpos);

//		worldPHY.update();
//		worldGFX.render();
//		worldPHY.render(cm);

        if(world.getInputHandler()->isFocused())
        {
            world.update();
            TwDraw();
		}

        window.display();
	}

	TwTerminate();

	delete wld;

    #ifdef SANIC_DEBUG
	Mesh::printMemoryStatistics();
	Texture::printMemoryStatistics();
	Shader::printMemoryStatistics();
    #endif // SANIC_DEBUG

	return EXIT_SUCCESS;
}
