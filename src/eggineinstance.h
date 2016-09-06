#ifndef EGGINEINSTANCE_H
#define EGGINEINSTANCE_H
#include <ostream>
#include <memory>
#include <string>

class World;
class Property;

class EggineInstance
{
    friend class Property;
public:
    typedef std::shared_ptr<EggineInstance> Ptr;

    virtual ~EggineInstance();
    virtual void setup() = 0;
    virtual int run() = 0;
    void setStartupWorld(std::string& path);
    static void printVersion(std::ostream& out);

protected:
    EggineInstance();
    void InitFs() const;
    std::string startupWorld;
    World* p_world;
};

#endif // EGGINEINSTANCE_H
