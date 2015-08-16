#include <catch.hpp>
#include "world.h"
#include "entity.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

TEST_CASE("Is parent serializing correctly?") {
    World w_1;
    Entity* d = w_1.createEntity("Decoration");
    Entity* dd = w_1.createEntity("Decoration");
    dd->setParent(d);
    rapidjson::Document dc;
    dd->Serialize(dc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    dc.Accept(writer);

    INFO("Serialized entity: \n" << buffer.GetString());
}
