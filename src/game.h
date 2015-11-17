#ifndef GAME_H
#define GAME_H
#include "eggineinstance.h"
#include <SFML/Graphics.hpp>

class World;
class InputHandler;

class Game : public EggineInstance
{
public:
    typedef std::shared_ptr<Game> Ptr;
    static Ptr Create();
    virtual ~Game();

    virtual void setup();
    int run();

private:
    Game();

    World* p_world;
	InputHandler* p_input;
    sf::RenderWindow* window;
};

#endif // GAME_H
