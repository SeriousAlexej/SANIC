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
typename std::enable_if<std::is_base_of<FromLua, C>::value>::type

Entity::addToLua(const std::string& s, C& c)
{
    private_lud.Set("get"+s, egg::getInstance().g_lua.CreateFunction<LuaUserdata<C>()>([&, s]() {
        auto lud = getProperty<C>(s).private_lud;
        return lud;
    }));
    private_lud.Set("set"+s, egg::getInstance().g_lua.CreateFunction<void(LuaUserdata<C>)>([&, s](LuaUserdata<C> arg) {
        setProperty<C>(s, *(arg.GetPointer()));
    }));
}

template<class C>
typename std::enable_if<
    TYPED(C, int) ||
    TYPED(C, float) ||
    TYPED(C, char) ||
    TYPED(C, double) ||
    TYPED(C, wchar_t) ||
    TYPED(C, std::string) ||
    TYPED(C, std::wstring) ||
    TYPED(C, bool)
>::type

Entity::addToLua(const std::string& s, C& c) {
    private_lud.Set("get"+s, egg::getInstance().g_lua.CreateFunction<C()>([&, s]() {
        return getProperty<C>(s);
    }));
    private_lud.Set("set"+s, egg::getInstance().g_lua.CreateFunction<void(C)>([&, s](C arg) {
        setProperty<C>(s, arg);
    }));
}

template<class C>
typename std::enable_if<
    NTYPED(C, int) &&
    NTYPED(C, float) &&
    NTYPED(C, char) &&
    NTYPED(C, double) &&
    NTYPED(C, wchar_t) &&
    NTYPED(C, std::string) &&
    NTYPED(C, std::wstring) &&
    NTYPED(C, bool) &&
    NBASED(C, FromLua)
>::type

Entity::addToLua(const std::string& s, C& c)
{
    private_lud.Set("get"+s, "Not available");
    private_lud.Set("set"+s, "Not available");
}

template<class C>
void Entity::registerProperties(const std::string& s, C* c, std::function<void(void)> cb)
{
    properties[s] = Property::create<C>(c, cb);
    addToLua(s, *c);
}

template<class C, class... T>
void Entity::registerProperties(const std::string& s, C* c, std::function<void(void)> cb, T... Args)
{
    registerProperties(s, c, cb);
    registerProperties(Args...);
}
