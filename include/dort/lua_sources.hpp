#include <unordered_map>
#include "dort/lua.hpp"

namespace dort {
  extern const std::unordered_map<std::string,
      std::pair<const char*,const char*>> lua_sources;
}
