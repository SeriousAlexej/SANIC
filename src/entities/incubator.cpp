#include <cassert>
#include "incubator.h"

Incubator* Incubator::instance = nullptr;

void Incubator::deleteInstance()
{
    if(instance != nullptr)
    {
        delete instance;
        instance = nullptr;
    }
}

void Incubator::addToCookBook(std::string className, size_t bytes, void (*ctorCaller)(void*))
{
    assert(ctorCaller != nullptr);
    assert(bytes > 0);
    if(instance == nullptr)
    {
        instance = new Incubator();
    }
    if(instance->cookBook.find(className) == instance->cookBook.end())
    {
        ClassInfo ci;
        ci.bytes = bytes;
        ci.ctorCaller = ctorCaller;
        instance->cookBook[className] = ci;
    }
}

void* Incubator::Create(std::string className)
{
    if(instance == nullptr)
    {
        instance = new Incubator();
    }
    if(instance->cookBook.find(className) == instance->cookBook.end())
    {
        throw std::bad_alloc();
    }
    ClassInfo &ci = instance->cookBook[className];
    void* ptr = ::operator new(ci.bytes);
    ci.ctorCaller(ptr);
    return ptr;
}
