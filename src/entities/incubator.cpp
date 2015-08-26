#include <cassert>
#include "incubator.h"
#include "entity.h"
#include <luauserdata.h>

Incubator& Incubator::getInstance()
{
    static Incubator instance;
    return instance;
}

void Incubator::addToCookBook(std::string className, size_t bytes, void (*ctorCaller)(void*))
{
    assert(ctorCaller != nullptr);
    assert(bytes > 0);
    if(getInstance().cookBook.count(className) == 0)
    {
        ClassInfo ci;
        ci.bytes = bytes;
        ci.ctorCaller = ctorCaller;
        getInstance().cookBook[className] = ci;
    }
}

void* Incubator::Create(const std::string& className)
{
    if(getInstance().cookBook.count(className) == 0)
    {
        throw not_in_cookbook();
    }
    ClassInfo &ci = getInstance().cookBook[className];
    void* ptr = ::operator new(ci.bytes);
    ci.ctorCaller(ptr);
    return ptr;
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
