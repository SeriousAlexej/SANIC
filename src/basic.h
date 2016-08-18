#ifndef _BASIC_H_
#define _BASIC_H_
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <list>
#include <string>
#include <algorithm>

#define TYPED(c, t) (std::is_same<t, c>::value)
#define NTYPED(c, t) (!std::is_same<t, c>::value)
#define BASED(c, t) (std::is_base_of<t, c>::value)
#define NBASED(c, t) (!std::is_base_of<t, c>::value)

class Movable
{
public:
    Movable();
    virtual ~Movable();

		const glm::vec3& getOffset() const;
		virtual glm::vec3 getPosition() const;
		const glm::mat4& getRotation() const;
		glm::quat getRotationQuat() const;
		glm::vec3 getScale() const;
		const glm::mat4& getMatrix() const;

		void  setPosition(glm::vec3 pos);
		void  setRotation(glm::vec3 rot);
		void  setRotation(glm::quat q);
		void  setRotation(glm::mat4 rot);
		void  setScale(glm::vec3 sca);
		void  setOffset(glm::vec3 off);

		void  translate(glm::vec3 addpos);
		void  rotate(glm::vec3 addrot);
		void  rotate(float angle, glm::vec3 dir);
		void  scale(glm::vec3 addsca);

protected:
    glm::vec3  offset;
    glm::mat4  posMx;
    glm::mat4  rotMx;
    glm::mat4  scaMx;
    glm::mat4  mdlMx;

 virtual void  updateModelMatrix();
};

class Subscribable
{
public:
    Subscribable() : subscribers(0) {}
    virtual ~Subscribable() = default;

    virtual void doSubscribe() { subscribers++; }
    virtual void unSubscribe()
    {
        subscribers--;
        if(subscribers <= 0)
        {
            allSubscribersLeft();
        }
    }
 virtual bool hasSubscribers() { return subscribers > 0; }

private:
    virtual void allSubscribersLeft() {}
    int  subscribers;
};

class Unique
{
public:
    Unique() { multipass = ++highestMultipass; }
    virtual ~Unique() {}

    int getMultipass() const { return multipass; }

    void setMultipass(int m)
    {
        multipass = m;
        if(m > highestMultipass) {
            highestMultipass = m;
        }
    }

private:
    static int highestMultipass;
    int  multipass;
};

//this class is used for storing results of 'custom callback' from Bullet
class Touchable
{
public:
    Touchable() { whoTouchedMe.clear(); }
    virtual ~Touchable() { whoTouchedMe.clear(); }

    virtual void touch(void* t)
    {
        if(t && std::find(whoTouchedMe.begin(), whoTouchedMe.end(), t) == whoTouchedMe.end())
            whoTouchedMe.push_back(t);
    }
    virtual void* popToucher() { void* r = whoTouchedMe.front(); whoTouchedMe.pop_front(); return r; }
    virtual bool lonely() { return whoTouchedMe.empty(); }

protected:
    std::list<void*> whoTouchedMe;
};

//this class should help with defining entity class
class FamilyTree
{
public:
    FamilyTree() : ancestorClasses(""), currentClass("") {}
    virtual ~FamilyTree() {}

    void _setClass(const std::string &myClass)
    {
        //can't have empty classname
        //can't have classname with '&'
        //can't be of the same class as one of the ancestors
        assert(myClass != "" &&
               myClass.find("&") == std::string::npos &&
               ancestorClasses.find(myClass) == std::string::npos);

        if(currentClass != "")
        {
            ancestorClasses += "&" + currentClass;
        }
        currentClass = myClass;
    }

    bool isDerivedFrom(const std::string& ancestor)
    {
        assert(ancestor != "");
        return currentClass == ancestor ||
               ancestorClasses.find(ancestor) != std::string::npos;
    }

    bool isOfClass(const std::string& classname)
    {
        assert(classname != "");
        return classname == currentClass;
    }

    const std::string& getClass() const { return currentClass; }

private:
    std::string  ancestorClasses;
    std::string  currentClass;
};

class FileName
{
public:
    FileName() : path("") {}
    FileName(const std::string& p) : path(p) {}
    ~FileName() = default;

    FileName& operator=(const std::string& p) { path = p; return *this; }
    FileName& operator=(const FileName& o) { path = o.path; return *this; }
    bool operator==(const std::string& p) const { return path == p; }
    bool operator==(const FileName& o) const { return path == o.path; }
    bool operator!=(const std::string& p) const { return !(*this == p); }
    bool operator!=(const FileName& o) const { return !(*this == o); }

    std::string path;
};

class Color
{
public:
    Color() : value(glm::vec3(1.0f, 1.0f, 1.0f)) {}
    Color(glm::vec3 c) : value(c) { Clamp(); }
    ~Color() = default;

    Color& operator=(glm::vec3 o)
    {
        value = o;
        Clamp();
        return *this;
    }
    Color& operator=(const Color& o) { value = o.value; return *this; }
    bool operator==(glm::vec3 o) const { return value == o; }
    bool operator==(const Color& o) const { return value == o.value; }
    bool operator!=(glm::vec3 o) const { return !(*this == o); }
    bool operator!=(const Color& o) const { return !(*this == o); }

    glm::vec3 value;

private:
    void Clamp()
    {
        value.x = glm::clamp(value.x, 0.0f, 1.0f);
        value.y = glm::clamp(value.y, 0.0f, 1.0f);
        value.z = glm::clamp(value.z, 0.0f, 1.0f);
    }
};

class FromLua
{
};

#endif
