#include <SFML/System.hpp>
#include <string>

char logo[] = {
"              _^  ^\n            _/  \\/ |\n        ___/ _/\\_\\/|\n     __/          \\|\n   _/         ====\\\\\n _/   _     //     |\n/____/     ||  ()  |\n  _/   __  ||      |\n /   _/   __\\\\____/\\()\n/___/    /__       /\n  _/     \\ \\__    /\n  /________\\_____/\n   _____ ___    _   ____________\n  / ___//   |  / | / /  _/ ____/\n  \\__ \\/ /| | /  |/ // // /     \n ___/ / ___ |/ /|  // // /___   \n/____/_/  |_/_/ |_/___/\\____/   \n"};

sf::Clock           g_Clock;
float               g_LastTime = 0.0f;
float               g_Delta = 0.0f;
bool	            g_Editor = false;
sf::Vector2u        g_Resolution(1024u, 768u);
sf::Vector2u        g_DrawOrigin(0u, 0u);
bool                g_UseDirectionalLight = true;
std::string         g_WorkingDir;
