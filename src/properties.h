#ifndef PROPERTIES
#define PROPERTIES

#include <exception>
#include <typeindex>
#include <functional>
#include <rapidjson/document.h>
#include "serial.h"

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
    size_t m_tid; // Typeid hash for our safety!
    std::function<void(void)> changeCallback;

    Property() : changeCallback(nullptr) {} // Private constructor
    ~Property() { }

public:
    template<class T>
    /**
	 * @brief Type-safe way to get value from property
	 *
	 * @return const T&
	 */
	const T& GetValue() const;

    template<class T>
    /**
     * @brief Sets value and emits a signal in SignalListener
	 *
	 * @param value Value
	 * @return void
	 */
	void SetValue(const T& value);

    template<class T>
    Property();

    template<class T>
    /**
	 * @brief Don't even know why would use this, but there is safe way to create property
	 *
	 * @param value Initial value
	 * @return Property
	 */
	static Property create(T* value, std::function<void(void)> cb);

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
	 * @brief Changes target pointer
	 *
	 * @param val New instance of property
	 * @return void
	 */
    void ChangeLocation(T* val);
};

template<class T>
const T &Property::GetValue() const {
    if(typeid(T).hash_code() != m_tid) throw std::bad_typeid();
    T* res = static_cast<T*>(m_data);
    return *res;
}

template<class T>
void Property::SetValue(const T& val) {
    if(typeid(T).hash_code() != m_tid) throw std::bad_typeid();
    T* t_data = static_cast<T*>(m_data);
    bool doCallback = *t_data != val;
    *t_data = val;
    if(changeCallback && doCallback)
        changeCallback();
}

template<class T>
Property Property::create(T* val, std::function<void(void)> cb) {
    Property ret;
    ret.changeCallback = cb;
    ret.m_tid = typeid(T).hash_code();
    ret.m_data = val;
    return ret;
}

template<class T>
void Property::ChangeLocation(T* val)
{
    if(val == nullptr) throw std::invalid_argument("Property is targeted at nullptr");
    if(typeid(T).hash_code() != m_tid) throw std::bad_typeid();
    this->m_data = val;
}

#endif // PROPERTIES

