#define CATCH_CONFIG_RUNNER
#include <string>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <AntTweakBar.h>
#include <boost/filesystem.hpp>
#include <entity.h>
#include <world.h>
#include <rapidjson/document.h>

#include <catch.hpp>

// + Entities sector
// TODO: fuck this
#include "entities/decoration.cpp"
// - Entities sector

sf::Clock g_Clock;
float	  g_LastTime = 0.0f;
float	  g_Delta = 0.0f;
bool	  g_Editor = false;
sf::Vector2u g_Resolution(1024u, 768u);
bool g_UseDirectionalLight = true;
std::string g_WorkingDir;

int main(int argc, char* const argv[])
{
    g_WorkingDir = boost::filesystem::current_path().string();
    std::replace(g_WorkingDir.begin(), g_WorkingDir.end(), '\\', '/');
    //g_Editor = (argc > 1);
    //printf(logo);
    //sf::sleep(sf::seconds(1.0f));

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
    glewInit();
    //TwInit(TW_OPENGL, NULL);
    //TwWindowSize(window.getSize().x,window.getSize().y);
    //TwCopyStdStringToClientFunc(CopyStdStringToClient);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	return Catch::Session().run(argc, argv);
}
