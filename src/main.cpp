#define CATCH_CONFIG_RUNNER
#include "editor.h"
#include "../test/Serialization.h"
#include <catch.hpp>
#include <SFML/Graphics.hpp>

int main(int argc, char **argv)
{
    // test shit
        sf::ContextSettings cs;
        cs.antialiasingLevel = 4;
        cs.depthBits = 24;
        cs.majorVersion = 3;
        cs.minorVersion = 3;

        sf::RenderWindow window(sf::VideoMode(800, 600), "Eggine Editor", sf::Style::Default, cs);
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

//    auto app = Editor::Create();
//    app->run();
    return Catch::Session().run(argc, argv);
}
