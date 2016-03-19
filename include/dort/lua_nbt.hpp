#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"

namespace dort {
  int lua_open_nbt(lua_State* l);
  int lua_nbt_read(lua_State* l);

  struct LuaNbtReader {
    lua_State* l;
    size_t length;
    const uint8_t* buffer;

    void read_named();
    void read_tag(int8_t type);
    std::string read_string(size_t length);

    template<class T, size_t Len, class R = T>
    T read_binary();

    uint8_t read_i8() { return this->read_binary<int8_t, 1>(); }
    int16_t read_i16() { return this->read_binary<int16_t, 2>(); }
    int32_t read_i32() { return this->read_binary<int32_t, 4>(); }
    int64_t read_i64() { return this->read_binary<int64_t, 8>(); }
    float read_float() { return this->read_binary<float, 4, int32_t>(); }
    double read_double() { return this->read_binary<double, 8, int64_t>(); }
  };
}
