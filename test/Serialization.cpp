#include <catch.hpp>
#include "world.h"
#include "entity.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <fstream>

#include <boost/crc.hpp>

boost::crc_32_type calc_crc(std::string filename)
{
    std::ifstream in(filename, std::ios_base::binary);
    boost::crc_32_type result;
    do {
        char buffer[1024];
        in.read(buffer, 1024);
        result.process_bytes(buffer, in.gcount());
    } while(in);
    return result;
}

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

TEST_CASE("Save/Love") {
    World w_1, w_2;
    Entity* d = w_1.createEntity("Decoration");
    Entity* dd = w_1.createEntity("Decoration");
    dd->setParent(d);
    w_1.Save("test_savelove_1.txt");
    w_2.Love("test_savelove_1.txt");
    w_2.Save("test_savelove_2.txt");
    //auto crc_1 = calc_crc("test_savelove_1.txt");
    //auto crc_2 = calc_crc("test_savelove_2.txt");
    //REQUIRE(crc_1.checksum() == crc_2.checksum());
    WARN("Check files manually!");
}

TEST_CASE("Lua boys") {
    World w;
    //w.createEntity("Decoration");
    std::ifstream in("test_lua_in.lua");
    std::stringstream buf;
    buf << in.rdbuf();
    w.getLua()->RunScript(buf.str());
    w.Save("test_lua.txt");
    WARN("Check file manually"); // TODO: it can be checked in code
}
