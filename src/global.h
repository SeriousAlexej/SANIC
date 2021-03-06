#ifndef GLOBAL_H
#define GLOBAL_H
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <luacppinterface.h>
#include <lua_utils.h>

class EggineInstance;

class egg {
public:
    const std::string   logo;
    sf::Clock           g_Clock;
    float               g_LastTime;
    float               g_Delta;
    bool                g_Editor;
    sf::Vector2u        g_Resolution = {800, 600};
    sf::Vector2u        g_DrawOrigin;
    bool                g_UseDirectionalLight = true;
    std::string         g_WorkingDir;
    Lua                 g_lua;
    EggineInstance*     g_Eggine;

    static egg& getInstance();
private:
    egg() :
    logo("              _^  ^\n            _/  \\/ |\n        ___/ _/\\_\\/|\n     __/          \\|\n   _/         ====\\\\\n _/   _     //     |\n/____/     ||  ()  |\n  _/   __  ||      |\n /   _/   __\\\\____/\\()\n/___/    /__       /\n  _/     \\ \\__    /\n  /________\\_____/\n   _____ ___    _   ____________\n  / ___//   |  / | / /  _/ ____/\n  \\__ \\/ /| | /  |/ // // /     \n ___/ / ___ |/ /|  // // /___   \n/____/_/  |_/_/ |_/___/\\____/   \n"),
    g_lua()
     {
        g_lua.GetGlobalEnvironment();
        g_lua.LoadStandardLibraries();
    }
    ~egg() {}
};

#endif // GLOBAL_H
