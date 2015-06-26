#ifndef PROPERTIES
#define PROPERTIES

#define if_type(X) if(typeid(X).hash_code() ==  m_tid)

#include <iostream>
#include <string>
#include <map>
#include <exception>
#include <typeindex>
#include <thread>
#include <rapidjson/document.h>

using namespace std;

typedef float quatfirst_t;

class Serial {
public:
    virtual void Deserialize(rapidjson::Value& d) = 0;
    virtual rapidjson::Value Serialize(rapidjson::Document& d) = 0;

    virtual ~Serial() {}
};

class Property : Serial {
private:

public:
    void* m_data; // UNSAFE!
    string m_name;
    size_t m_tid; // Typeid hash for our safety!

    Property() {}; // Private constructor
    ~Property() { };

public:
    template<class T>
    T& GetValue() const;

    template<class T>
    void SetValue(T val);

    template<class T>
    Property();

    template<class T>
    static Property* create(T* val);
    
    rapidjson::Value Serialize(rapidjson::Document& d);
                void Deserialize(rapidjson::Value& d);

    template<class T>
    T& ref();

    template<class T>
    void ChangeLocation(T* val);
};

template<class T>
T &Property::GetValue() const {
    if(typeid(T).hash_code() != m_tid) throw bad_typeid();
    T* res = static_cast<T*>(m_data);
    return *res;
}

template<class T>
void Property::SetValue(T val) {
    if(typeid(T).hash_code() != m_tid) throw bad_typeid();
    T* t_data = static_cast<T*>(m_data);
    *t_data = val;
}

template<class T>
Property* Property::create(T* val) {
    Property* ret = new Property;
    ret->m_tid = typeid(T).hash_code();
    ret->m_data = val;
    return ret;
}

template<class T>
void Property::ChangeLocation(T* val)
{
    if(val == nullptr) throw invalid_argument("Property is targeted at nullptr");
    if(typeid(T).hash_code() != m_tid) throw bad_typeid();
    this->m_data = val;
}

template<class T>
T& Property::ref()
{
    if(typeid(T).hash_code() != m_tid) throw bad_typeid();
    T* t_data = static_cast<T*>(m_data); // TODO: don't work
    return *t_data;
}

#endif // PROPERTIES

