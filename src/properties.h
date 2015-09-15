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

/**
 * @brief Objects of this class can be (de)serialized into JSON
 * 
 */
class Serial {
public:
	/**
	 * @brief Deserializes object from JSON value
	 * 
	 * @param value JSON value
	 * @return void
	 */
	virtual void Deserialize(rapidjson::Value& value) = 0;
	/**
	 * @brief Serializes class into JSON objects
	 * 
	 * @param document Document object, need for encoding and stuff
	 * @return rapidjson::Value
	 */
	virtual rapidjson::Value Serialize(rapidjson::Document& document) = 0;

    virtual ~Serial() {}
};

/**
 * @brief Powerful tool for storing properties that can be serialized and exported in Lua.
 * 
 */
class Property : Serial {
private:

public:
	/**
	 * @brief There can be *everything*.
	 * Don't even try to work with it manually.
	 * Use GetValue and SetValue instead for type-safety
	 * 
	 */
	void* m_data;
    string m_name;
    size_t m_tid; // Typeid hash for our safety!

    Property() {}; // Private constructor
    ~Property() { };

public:
    template<class T>
    /**
	 * @brief Type-safe way to get value from property
	 * 
	 * @return T&
	 */
	T& GetValue() const;

    template<class T>
    /**
	 * @brief Sets value
	 * TODO: emitting a special signal in SignalListener
	 * 
	 * @param value Value
	 * @return void
	 */
	void SetValue(T& value);

    template<class T>
    Property();

    template<class T>
    /**
	 * @brief Don't even know why would use this, but there is safe way to create property
	 * 
	 * @param value Initial value
	 * @return Property*
	 */
	static Property* create(T* value);
    
	/**
	 * @brief Property-specific serialization
	 * 
	 * @param document Same as in [Serial](@ref Serial)
	 * @return rapidjson::Value
	 */
	rapidjson::Value Serialize(rapidjson::Document& document);
	/**
	 * @brief Deserializes JSON vaue into the property
	 * 
	 * @param value Same as in [Serial](@ref Serial)
	 * @return void
	 */
	void Deserialize(rapidjson::Value& value);

    template<class T>
    /**
	 * @brief Deprecated
	 * 
	 * @return T&
	 */
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
void Property::SetValue(T& val) {
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

