#ifndef PROPERTIES
#define PROPERTIES

#include <iostream>
#include <string>
#include <map>
#include <exception>
#include <typeindex>
#include <thread>

using namespace std;

typedef float quatfirst_t;

class Serial {
public:
    virtual void Deserialize(istream& ostr) = 0;
    virtual void Serialize(ostream& istr) = 0;

    virtual ~Serial() {}
};

class Property {
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

    template<class T>
    void Serialize(ostream& o);

    template<class T>
    void Deserialize(istream& o);

    template<class T>
    T& ref();
};

template<class T>
T &Property::GetValue() const {
    if(typeid(T).hash_code() != m_tid) throw bad_typeid();
    T res = *reinterpret_cast<T*>(m_data);
    return res;
}

template<class T>
void Property::SetValue(T val) {
    if(typeid(T).hash_code() != m_tid) throw bad_typeid();
    T* t_data = reinterpret_cast<T*>(m_data);
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
void Property::Serialize(ostream &o)
{
    if(typeid(T).hash_code() != m_tid) throw bad_typeid();
    T* t_data = reinterpret_cast<T*>(m_data);
    Serial* ser;
    try {
        ser = dynamic_cast<Serial*>(t_data);
        ser->Serialize(o);
    }
    catch(exception& e) {
        cout << "Property is not serial!" << endl;
    }
}

template<class T>
void Property::Deserialize(istream &o)
{
    if(typeid(T).hash_code() != m_tid) throw bad_typeid();
    T* t_data = reinterpret_cast<T*>(m_data);
    Serial* ser;
    try {
        ser = dynamic_cast<Serial*>(t_data);
        ser->Deserialize(o);
    }
    catch(exception& e) {
        cout << "Property is not serial!" << endl;
    }
}


template<class T>
T& Property::ref()
{
    if(typeid(T).hash_code() != m_tid) throw bad_typeid();
    T* t_data = reinterpret_cast<T*>(m_data); // TODO: don't work
    return *t_data;
}

#endif // PROPERTIES

