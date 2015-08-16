#include <cassert>
#include "incubator.h"

Incubator& Incubator::getInstance()
{
    static Incubator instance;
    return instance;
}

void Incubator::addToCookBook(std::string className, size_t bytes, void (*ctorCaller)(void*))
{
    assert(ctorCaller != nullptr);
    assert(bytes > 0);
    if(getInstance().cookBook.find(className) == getInstance().cookBook.end())
    {
        ClassInfo ci;
        ci.bytes = bytes;
        ci.ctorCaller = ctorCaller;
        getInstance().cookBook[className] = ci;
    }
}

void* Incubator::Create(std::string className)
{
    if(getInstance().cookBook.find(className) == getInstance().cookBook.end())
    {
        throw not_in_cookbook();
    }
    ClassInfo &ci = getInstance().cookBook[className];
    void* ptr = ::operator new(ci.bytes);
    ci.ctorCaller(ptr);
    return ptr;
}
