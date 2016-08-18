#ifndef LUA_UTILS
#define LUA_UTILS

#define LUA_GETTER(l, x, y, z) \
l.CreateFunction<x()>([&, y]() { \
    return y->z; \
})

#define LUA_SETTER(l, x, y, z) \
l.CreateFunction<void(x)>([&, y](x a) { \
    y->z = a; \
})

#define LUA_UGETTER(l, x, y, z) \
l.CreateFunction<LuaUserdata<x>()>([&, y]() { \
    auto eud = y->z->private_lud; \
    return eud; \
})

#define LUA_USETTER(l, x, y, z) \
l.CreateFunction<void(LuaUserdata<x>)>([&, y](LuaUserdata<x> a) { \
    y->z = a.GetPointer(); \
})

#endif // LUA_UTILS
