#include <string>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <SFML/Graphics.hpp>

#include "world.h"

sf::Clock g_Clock;
float	  g_LastTime = 0.0f;
float	  g_Delta = 0.0f;

const static char logo[] = {
"              _^  ^\n            _/  \\/ |\n        ___/ _/\\_\\/|\n     __/          \\|\n   _/         ====\\\\\n _/   _     //     |\n/____/     ||  ()  |\n  _/   __  ||      |\n /   _/   __\\\\____/\\()\n/___/    /__       /\n  _/     \\ \\__    /\n  /________\\_____/\n   _____ ___    _   ____________\n  / ___//   |  / | / /  _/ ____/\n  \\__ \\/ /| | /  |/ // // /     \n ___/ / ___ |/ /|  // // /___   \n/____/_/  |_/_/ |_/___/\\____/   \n"};


void measureFPS()
{
	static sf::Clock clock;
	static int nbFrames = 0;
	static double lastTime = clock.getElapsedTime().asSeconds();
	double currTime = clock.getElapsedTime().asSeconds();
	nbFrames++;
	float delta = currTime - lastTime;
	if(delta >= 1.0)
	{
#ifdef SANIC_DEBUG
		printf("%f ms/frame\n", 1000.0/double(nbFrames));
#endif
		nbFrames = 0;
		lastTime += 1.0;
	}
}

int main()
{
	printf(logo);
	sf::sleep(sf::seconds(1.0f));

	sf::ContextSettings cs;
	cs.antialiasingLevel = 4;
	cs.depthBits = 32;
	cs.majorVersion = 2;
	cs.minorVersion = 1;
	sf::RenderWindow window(sf::VideoMode(1024, 768), "SANIC", sf::Style::Default, cs);
    window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);
	
    // Make it the active window for OpenGL calls
    window.setActive();
	
	// Initialise GLEW
	glewExperimental=true;
	if(glewInit() != GLEW_OK)
	{
		fprintf( stderr, "Failed to initialize GLEW\n" );
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);

	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

	World world;
	LiveEntity* e = (LiveEntity*)world.createEntity((Entity*)(new LiveEntity()));

	//test
	//e->setupModel("./shaders/smooth.vsh", "./shaders/smooth.fsh",
	//										 "./models/sanic.obj",
	//										 "./models/sanic.tga", "./models/normal.jpg");
	e->setDesiredRotation(glm::vec3(0,3.14,0));
	e->setRotationSpeed(0.5f);


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
	float speed = 3.0f;
	float mouseSpeed = 0.05f;

	bool wasPressed = false;

	g_Clock.restart();
    // Start game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window : exit
            if (event.type == sf::Event::Closed)
                window.close();

            // Escape key : exit
            if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape))
                window.close();

            // Adjust the viewport when the window is resized
            if (event.type == sf::Event::Resized)
                glViewport(0, 0, event.size.width, event.size.height);
        }
		//measureFPS();

		float currTime = g_Clock.getElapsedTime().asSeconds();
		g_Delta = currTime - g_LastTime;
		g_LastTime = currTime;

		sf::Vector2i mpos = sf::Mouse::getPosition(window);
		sf::Mouse::setPosition(sf::Vector2i(1024/2,768/2), window);

		//cm->rotate(mouseSpeed * g_Delta * float(1024/2 - mpos.x ),
		//	       mouseSpeed * g_Delta * float( 768/2 - mpos.y ));


		if (sf::Keyboard::isKeyPressed(sf::Keyboard::K)){
			/*
			if(axis != NULL)
			{
				worldGFX.deleteModel(axis);
			}
			if(cube)
			{
				worldPHY.remBody(cube);
			}*/
			e->switchToModel();
			wasPressed = true;
		} else
		if(wasPressed)
		{
			e->sendEvent(new EntityEvent());
			wasPressed = false;
		}


		glm::vec3 newpos(0,0,0);
		// Move forward
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
			newpos.x += (g_Delta * -speed);
		} else
		// Move backward
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
			newpos.x += (g_Delta * speed);
		}
		// Strafe right
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
			newpos.z += (g_Delta * speed);
		} else
		// Strafe left
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
			newpos.z += (g_Delta * -speed);
		}

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

		world.update();

        window.display();
	}

	return EXIT_SUCCESS;
}