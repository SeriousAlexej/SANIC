#ifndef INCUBATOR_H
#define INCUBATOR_H
#include <map>
#include <string>

class not_in_cookbook : public std::exception
{
    virtual const char* what() const throw()
    {
        return "No such entity in cookbook";
    }
};

class Incubator
{
    public:
        static void* Create(std::string className);
        static void addToCookBook(std::string className, size_t bytes, void (*ctorCaller)(void*));
        static Incubator& getInstance(); // Trash

    private:
        Incubator() {}
        ~Incubator() {}

        struct ClassInfo
        {
            size_t bytes;
            void (*ctorCaller)(void*);
        };
        std::map<std::string,ClassInfo> cookBook;
};

class FromIncubator
{
public:
    FromIncubator() : ptr(nullptr) {}
    virtual ~FromIncubator() {}
protected:
    void* ptr;

    friend class World;
};

#define FROM_INCUBATOR(x) \
static void constructorCaller(void *p) \
{ \
    new (p) x(); \
    x* dp = static_cast<x*>(p); \
    dp->ptr = p; \
} \
static class _init \
{ \
  public: \
    _init() \
    { \
        Incubator::addToCookBook(#x, sizeof(x), &(x::constructorCaller)); \
    } \
} _initializer;

#define ADD_TO_INCUBATOR(x) \
x::_init x::_initializer

#endif // INCUBATOR_H
