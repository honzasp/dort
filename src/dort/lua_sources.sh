#!/bin/bash
# The Lua sources from lua/ are compiled using luac and converted into object
# files which contain the Lua bytecode in their .rodata section, bracketed by a
# pair of symbols pointing to the beginning and end of the block.
# This script is given a list of Lua source names accompanied by the names of
# the begin and end symbols, and produces a C++ file that defines the variable
# lua_sources declared in lua_sources.hpp.

printf "// this file was generated automatically by %s\n" "$0"
cat <<END
#include <unordered_map>
//#include "dort/lua_sources.hpp"

namespace dort {
  extern "C" {
END

args=( "$@" )

i=0
while [ $i -lt ${#args[*]} ]; do
  sym_begin="${args[i+1]}"
  sym_end="${args[i+2]}"
  printf '    extern char %s;\n' "$sym_begin" "$sym_end"
  i=$((i+3))
done

cat <<END
  }

  extern const std::unordered_map<std::string,
    std::pair<const char*, const char*>> lua_sources = 
  {
END

i=0
while [ $i -lt ${#args[*]} ]; do
  id="${args[i]}"
  sym_begin="${args[i+1]}"
  sym_end="${args[i+2]}"
  printf '    {"%s", {\n      &%s,\n      &%s}},\n' "$id" "$sym_begin" "$sym_end"
  i=$((i+3))
done

cat <<END
  };
}
END
