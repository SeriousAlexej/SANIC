#ifndef INCUBATOR_H
#define INCUBATOR_H
#include <map>
#include <vector>
#include <string>
#include <luacppinterface.h>
#include <functional>
#include <memory>
#include <entity.h>

class not_in_cookbook : public std::exception
{
    virtual const char* what() const throw()
    {
        return "No such entity in cookbook";
    }
};
class cant_create : public std::exception
{
    virtual const char* what() const throw()
    {
        return "Can't create entity";
    }
};

class Incubator
{
    public:
        static std::shared_ptr<Entity> Create(const std::string &className);
        static void addToCookBook(std::string className, std::function<std::shared_ptr<Entity>()> ctorCaller);
        static Incubator& getInstance();
        static std::vector<std::string> getRegisteredClasses();

    private:
        Incubator() {}
        ~Incubator() {}

        struct ClassInfo
        {
            std::function<std::shared_ptr<Entity>()> ctorCaller;
        };
        std::map<std::string,ClassInfo> cookBook;
        friend class World;
};

#define FROM_INCUBATOR(x) \
static std::shared_ptr<Entity> constructorCaller() \
{ \
    std::shared_ptr<Entity> p = std::make_shared<x>(); \
    p->_setClass(#x); \
    return p; \
} \
static class _init \
{ \
  public: \
    _init() \
    { \
        Incubator::addToCookBook(#x, &(x::constructorCaller)); \
    } \
} _initializer;

#define ADD_TO_INCUBATOR(x) \
x::_init x::_initializer

#endif // INCUBATOR_H
