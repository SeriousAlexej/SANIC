#ifndef EGGINEINSTANCE_H
#define EGGINEINSTANCE_H
#include <ostream>
#include <memory>
#include <string>

class EggineInstance
{
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
};

#endif // EGGINEINSTANCE_H
