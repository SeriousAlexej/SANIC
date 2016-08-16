template<class C>
void Entity::registerProperties(const std::string& s, C* c, std::function<void(void)> cb)
{
    properties[s] = Property::create<C>(c, cb);
}

template<class C, class... T>
void Entity::registerProperties(const std::string& s, C* c, std::function<void(void)> cb, T... Args)
{
    registerProperties(s, c, cb);
    registerProperties(Args...);
}

template<class T>
const T& Entity::getProperty(const std::string& name)
{
    return properties[name].GetValue<T>();
}

template<class T>
void Entity::setProperty(const std::string& s, const T& val)
{
    properties[s].SetValue<T>(val);
}

template<class C>
typename std::enable_if<std::is_base_of<FromLua, C>::value>::type Entity::addToLua(LuaUserdata<Entity>& l, const std::string& s, C c)
{
    Lua *pLua = &egg::getInstance().g_lua;
    l.Set("get"+s, pLua->CreateFunction<LuaUserdata<C>()>([&, s]() {
        auto lud = pLua->CreateUserdata<C>(&(getProperty<C>(s)));
        if(lud.GetPointer() != nullptr) lud->registerLua(lud);
        return lud;
    }));
    l.Set("set"+s, pLua->CreateFunction<void(LuaUserdata<C>)>([&, s](LuaUserdata<C> arg) {
        setProperty<C>(s, *(arg.GetPointer()));
    }));
}

template<class C>
typename std::enable_if<!std::is_base_of<FromLua, C>::value>::type Entity::addToLua(LuaUserdata<Entity>& l, const std::string& s, C c)
{
    Lua *pLua = &egg::getInstance().g_lua;
    l.Set("get"+s, pLua->CreateFunction<C()>([&, s]() {
        return getProperty<C>(s);
    }));
    l.Set("set"+s, pLua->CreateFunction<void(C)>([&, s](C arg) {
        setProperty<C>(s, arg);
    }));
}

template<class C, class... T>
void Entity::addToLua(LuaUserdata<Entity> &l, const std::string& s, C c, T... args)
{
    addToLua(l, s, c);
    addToLua(l, args...);
}
