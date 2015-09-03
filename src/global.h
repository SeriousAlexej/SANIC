#ifndef GLOBAL_H
#define GLOBAL_H
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <luacppinterface.h>

class egg {
public:
    char          logo[];
    sf::Clock           g_Clock;
    float               g_LastTime;
    float               g_Delta;
    bool                g_Editor; // TODO: remove
    sf::Vector2u        g_Resolution = {800, 600};
    sf::Vector2u        g_DrawOrigin;
    bool                g_UseDirectionalLight = true;
    std::string         g_WorkingDir;
    Lua                 g_lua;

    static egg& getInstance();
private:
    egg() : g_lua() {
        g_lua.GetGlobalEnvironment();
        g_lua.LoadStandardLibraries();
    }
    ~egg() {}
};

#endif // GLOBAL_H
