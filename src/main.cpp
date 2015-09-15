#include <string>
#include <iostream>
#include "editor.h"
#include "game.h"
#include <catch.hpp>
//#include "../test/Serialization.h"

unsigned char flagEditor = 1 << 0,
              flagWorld  = 1 << 1;

int main(int argc, char **argv)
{
    std::string helpString = std::string("Eggine arguments:\n") +
                                            "\tLoad world: -w <path> or --world <path>\n" +
                                            "\tLaunch editor: -e or --editor\n" +
                                            "\tDisplay version: -v or --version\n" +
                                            "\tShow this help: -h or --help\n";
    unsigned char appFlags = 0;
    std::string wld = "";
    EggineInstance::Ptr app;

    for(int i=1; i<argc; ++i)
    {
        std::string arg = argv[i];
        if(arg == "-h" || arg == "--help")
        {
            std::cout << helpString;
            return EXIT_SUCCESS;
        } else
        if(arg == "-e" || arg == "--editor")
        {
            appFlags |= flagEditor;
        } else
        if(arg == "-w" || arg == "--world")
        {
            if(i >= argc - 1)
            {
                std::cout << "Too few arguments for " + arg << std::endl;
            } else {
                wld = argv[++i];
                appFlags |= flagWorld;
            }
        } else
        if(arg == "-v" || arg == "--version")
        {
            EggineInstance::printVersion(std::cout);
            return EXIT_SUCCESS;
        } else
        {
            std::cout << "Unknown argument " + arg  << std::endl;
            continue;
        }
    }

    if(appFlags & flagEditor)
    {
        app = Editor::Create();
    } else {
        app = Game::Create();
    }

    if(appFlags & flagWorld)
    {
        app->setStartupWorld(wld);
    }
    app->setup();
#ifdef SANIC_TEST
    Catch::Session().run(argc, argv);
#endif
    return app->run();
}
