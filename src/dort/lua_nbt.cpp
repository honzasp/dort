/// NBT (binary format used by Minecraft).
// NBT (Named Binary Tag) is documented at
// http://minecraft.gamepedia.com/NBT_format
// @module dort.nbt
#include "dort/lua_nbt.hpp"

namespace dort {
  int lua_open_nbt(lua_State* l) {
    const luaL_Reg nbt_funs[] = {
      {"read", lua_nbt_read},
      {0, 0},
    };
    luaL_newlib(l, nbt_funs);
    return 1;
  }

  /// Parse the NBT data in `str`.
  // The NBT types are mapped to their Lua equivalents
  //
  // - all integers and floats are mapped to Lua numbers
  // - strings and byte arrays are mapped to Lua strings
  // - lists and objects are mapped to Lua tables
  // - int array is mapped to Lua table
  //
  // @function read
  // @param str
  int lua_nbt_read(lua_State* l) {
    size_t data_len;
    const char* data_buf = luaL_checklstring(l, 1, &data_len);

    LuaNbtReader reader;
    reader.l = l;
    reader.length = data_len;
    reader.buffer = (uint8_t*)(data_buf);

    reader.read_named();
    return 1;
  }

  void LuaNbtReader::read_named() {
    int8_t tag_type = this->read_i8();
    if(tag_type == 0) {
      lua_pushnil(this->l);
      return;
    }

    int16_t name_len = this->read_i16();
    std::string name_data = this->read_string(name_len);
    lua_pushlstring(this->l, name_data.data(), name_data.size());
    this->read_tag(tag_type);
  }

  void LuaNbtReader::read_tag(int8_t type) {
    if(type == 0) {
      lua_pushnil(this->l);
    } else if(type == 1) {
      lua_pushinteger(this->l, this->read_i8());
    } else if(type == 2) {
      lua_pushinteger(this->l, this->read_i16());
    } else if(type == 3) {
      lua_pushinteger(this->l, this->read_i32());
    } else if(type == 4) {
      lua_pushinteger(this->l, this->read_i64());
    } else if(type == 5) {
      lua_pushnumber(this->l, this->read_float());
    } else if(type == 6) {
      lua_pushnumber(this->l, this->read_double());
    } else if(type == 7 || type == 8) {
      int32_t length = type == 7 ? this->read_i32() : this->read_i16();
      if(length < 0) {
        luaL_error(this->l, "Array length must be non-negative");
      }

      std::string data(this->read_string(length));
      lua_pushlstring(this->l, data.data(), data.size());
    } else if(type == 9) {
      int8_t list_type = this->read_i8();
      int32_t list_length = this->read_i32();
      if(list_length < 0) {
        luaL_error(this->l, "List length must be non-negative");
      }

      lua_createtable(this->l, list_length, 0);
      lua_checkstack(this->l, 10);
      for(int32_t i = 0; i < list_length; ++i) {
        this->read_tag(list_type);
        lua_rawseti(this->l, -2, i + 1);
      }
    } else if(type == 10) {
      lua_createtable(this->l, 0, 0);
      lua_checkstack(this->l, 10);
      for(;;) {
        this->read_named();
        if(lua_isnil(this->l, -1)) {
          lua_pop(this->l, 1);
          break;
        }
        lua_rawset(this->l, -3);
      }
    } else if(type == 11) {
      int32_t array_length = this->read_i32();
      if(array_length < 0) {
        luaL_error(this->l, "Array length must be non-negative");
      }

      lua_createtable(this->l, array_length, 0);
      for(int32_t i = 0; i < array_length; ++i) {
        lua_pushinteger(this->l, this->read_i32());
        lua_rawseti(this->l, -2, i + 1);
      }
    } else {
      luaL_error(this->l, "Unknown NBT type: %d", type);
    }
  }

  std::string LuaNbtReader::read_string(size_t length) {
    if(this->length < length) {
      luaL_error(this->l, "Unexpected end of file");
    }
    std::string str((const char*)this->buffer, length);
    this->buffer += length;
    this->length -= length;
    return str;
  }

  template<class T, size_t Len, class R>
  T LuaNbtReader::read_binary() {
    if(this->length < Len) {
      luaL_error(this->l, "Unexpected end of file");
    }
    union {
      T payload;
      R integer;
    } bitcast;
    for(size_t i = 0; i < Len; ++i) {
      bitcast.integer = (bitcast.integer << 8) + this->buffer[i];
    }
    this->buffer += Len;
    this->length -= Len;
    return bitcast.payload;
  }
}
