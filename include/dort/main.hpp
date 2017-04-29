#pragma once
#include "dort/dort.hpp"

namespace dort {
  extern const char* const SHORT_USAGE;
  extern const char* const LONG_USAGE;
  int main(uint32_t argc, char** argv);
  int main_lua(lua_State* l, const char* input_file);
}

int main(int argc, char** argv);
