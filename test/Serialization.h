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

TEST_CASE("Lua constructor") {
    World w;
    //w.createEntity("Decoration");
    w.getLua()->RunScript("Decoration.new()");
    REQUIRE(w.getEntities().size() == 1);
}

TEST_CASE("Lua get/set") {
    World w;
    w.getLua()->RunScript("local dec = Decoration.new()"
                          "dec.setName('dick')"
                          "local nm = dec.getName()"
                          "dec.setName(string.upper(nm))");
    Entity* pen = w.getEntities()[0];
    REQUIRE(pen->getName() == "DICK");
}

TEST_CASE("Lua events") {
    World w;
    w.getLua()->RunScript("local ev = EntityEvent.new('Damage')"
                          "print(ev.getAmount())"
                          "local dec = Decoration.new()"
                          "dec.setName('dick')"
                          "ev.setDamager(dec)"
                          "ev.getDamager().setName('not dick!')");
    Entity* pen = w.getEntities()[0];
    REQUIRE(pen->getName() == "not dick!");
}
