#include <boost/filesystem.hpp>
#include "eggineinstance.h"
#include "global.h"

EggineInstance::EggineInstance()
{
    InitFs();
    startupWorld = "";
}

EggineInstance::~EggineInstance()
{
}

void EggineInstance::printVersion(std::ostream& out)
{
    out << "Eggine build " << __DATE__ << "@" << __TIME__ << std::endl;
}

void EggineInstance::setStartupWorld(std::string& path)
{
    startupWorld = path;
}

void EggineInstance::InitFs() const
{
    egg::getInstance().g_WorkingDir = boost::filesystem::current_path().string();
    std::replace(egg::getInstance().g_WorkingDir.begin(), egg::getInstance().g_WorkingDir.end(), '\\', '/');
}
