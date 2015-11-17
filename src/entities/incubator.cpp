#include <cassert>
#include "incubator.h"
#include "entity.h"
#include <luauserdata.h>

Incubator& Incubator::getInstance()
{
    static Incubator instance;
    return instance;
}

void Incubator::addToCookBook(std::string className, std::function<std::shared_ptr<Entity>()> ctorCaller)
{
    assert(ctorCaller != nullptr);
    if(getInstance().cookBook.count(className) == 0)
    {
        ClassInfo ci;
        ci.ctorCaller = ctorCaller;
        getInstance().cookBook[className] = ci;
    }
}

std::shared_ptr<Entity> Incubator::Create(const std::string& className)
{
    if(getInstance().cookBook.count(className) == 0)
    {
        throw not_in_cookbook();
    }
    ClassInfo &ci = getInstance().cookBook[className];
    return ci.ctorCaller();
}

std::vector<std::string> Incubator::getRegisteredClasses()
{
    std::vector<std::string> ret;
    std::map<std::string, ClassInfo> &mp = getInstance().cookBook;
    for(auto it = mp.begin(); it != mp.end(); ++it)
    {
        ret.push_back(it->first);
    }
    return ret;
}
