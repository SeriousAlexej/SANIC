/*
#include <catch.hpp>
#include <entity.h>
#include <world.h>
#include <rapidjson/document.h>
#include <SFML/Graphics.hpp>

TEST_CASE("Is parent serializing correctly?") {
    sf::RenderWindow win; // Fuck this shit
    win.setActive();
	World w(&win);
	Entity* d = w.createEntity("Decoration");
    Entity* dd = w.createEntity("Decoration");
    dd->setParent(d);
	rapidjson::Document dc;
	dd->Serialize(dc);
    INFO("Serialized entity: \n" << dc.GetString());
}
*/
