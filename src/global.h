#ifndef GLOBAL_H
#define GLOBAL_H
#include <SFML/System/Clock.hpp>
#include <string>

extern char                logo[];
extern sf::Clock           g_Clock;
extern float               g_LastTime;
extern float               g_Delta;
extern bool	               g_Editor;
extern sf::Vector2u        g_Resolution;
extern sf::Vector2u        g_DrawOrigin;
extern bool                g_UseDirectionalLight;
extern std::string         g_WorkingDir;

#endif // GLOBAL_H
