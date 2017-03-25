local cxx = "clang++"
local objcopy = "objcopy"
local pkg_config = "pkg-config"

local includes = "-Iinclude -Iextern/rply -Iextern/stb -Iextern/lua -Iextern/lgi"
local warnings = "-Wall -Wextra -Wstrict-aliasing"
local cxxflags = "-x c++ -std=c++1y -pthread "..includes.." "..warnings
local cflags   = "-x c -std=c11 -pthread "..includes.." "..warnings
local ldflags  = "-pthread -lz -static-libstdc++ -static-libgcc"
local target_flags = {
  debug = "-g -O2 -DLUA_USE_APICHECK",
  slow  = "-g3 -O0 -DLUA_USE_APICHECK",
  fast  = "-g -O3 -DNDEBUG -DDORT_DISABLE_STAT",
}

local use_gtk = tup.getconfig("USE_GTK") != ""

if use_gtk then
  local pkgs = "gobject-introspection-1.0 gmodule-2.0 gio-2.0 gdk-pixbuf-2.0 libffi"
  local pkg_libs = string.format("`%s --libs %s`", pkg_config, pkgs)
  local pkg_cflags = string.format("`%s --cflags %s`", pkg_config, pkgs)
  ldflags = ldflags .. " " .. pkg_libs
  cxxflags = cxxflags .. " -DDORT_USE_GTK -Wno-deprecated-register " .. pkg_cflags
  cflags = cflags .. " -DDORT_USE_GTK " .. pkg_cflags
end

local build_dirs = {
  debug = "build/d",
  slow = "build/g",
  fast = "build/f",
}

local source_dirs = {
  {"c++", "src/dort/*.cpp", "dort"},
  {"c", "extern/rply/*.c", "rply"},
  {"c", "extern/lua/*.c", "lua"},
  {"c", "extern/luac/*.c", "luac"},
}
if use_gtk then
  source_dirs[#source_dirs + 1] = {"c", "extern/lgi/lgi/*.c", "lgi"}
end

local lua_dir = "lua"
local lua_subdirs = {
  "", "dort", "minecraft",
  "extern", "extern/lgi", "extern/lgi/override",
}

function compile_target(target)
  local build_dir = build_dirs[target]

  function compile(lang, input_source, output_obj)
    local flags
    if lang == "c++" then
      flags = cxxflags .. " " .. target_flags[target]
    elseif lang == "c" then
      flags = cflags .. " " .. target_flags[target]
    else
      error(lang)
    end

    tup.definerule {
      command = string.format("^ %s %s %s^ %s -c %s %s -o %s",
        lang, target, input_source,
        cxx, flags, input_source, output_obj),
      inputs = {input_source},
      outputs = {output_obj},
    }
  end

  function link(input_objs, output_exec)
    tup.definerule {
      command = string.format("^ link %s %s^ %s %s -o %s %s",
        target, output_exec,
        cxx, table.concat(input_objs, " "), output_exec, ldflags),
      inputs = input_objs,
      outputs = {output_exec},
    }
  end

  local dort_objs = {}
  local dort_cat_cxxs = {}
  local luac_objs = {}

  for _, row in ipairs(source_dirs) do
    local lang = row[1]
    local input_glob = row[2]
    local subdir = row[3]

    for _, input_file in ipairs(tup.glob(input_glob)) do
      if target == "fast" and string.find(subdir, "dort") then
        dort_cat_cxxs[#dort_cat_cxxs + 1] = input_file
        goto skip_obj
      end

      local output_obj = string.format("%s/o/%s/%s.o",
        build_dir, subdir, tup.base(input_file))
      compile(lang, input_file, output_obj)

      if subdir != "luac" then
        dort_objs[#dort_objs + 1] = output_obj
      end
      if subdir == "luac" or subdir == "lua" then
        luac_objs[#luac_objs + 1] = output_obj
      end
      ::skip_obj::
    end
  end

  function cat_cxxs(input_sources, output_cxx)
    tup.definerule {
      command = string.format("^ cat_cxxs.sh %s^ sh script/cat_cxxs.sh %s >%s",
        target, table.concat(input_sources, " "), output_cxx),
      inputs = input_sources,
      outputs = {output_cxx},
    }
  end

  if #dort_cat_cxxs > 0 then
    local cat_cxx = string.format("%s/dort_cat.cpp", build_dir)
    local cat_obj = string.format("%s/o/dort_cat.o", build_dir)
    cat_cxxs(dort_cat_cxxs, cat_cxx)
    compile("c++", cat_cxx, cat_obj)
    dort_objs[#dort_objs + 1] = cat_obj
  end

  local luac_exec = string.format("%s/luac_exec", build_dir)
  link(luac_objs, luac_exec)

  function compile_lua(input_lua, output_luac)
    tup.definerule {
      command = string.format("^ luac %s %s^ %s -o %s %s",
        target, input_lua,
        luac_exec, output_luac, input_lua),
      inputs = {input_lua, luac_exec},
      outputs = {output_luac},
    }
  end

  function binary_file_to_obj(input_file, output_obj)
    tup.definerule {
      command = string.format("^ objcopy %s %s^" ..
        "%s -I binary -O elf64-x86-64 -B i386 " ..
        "--rename-section .data=.rodata,readonly,data,contents %s %s",
        target, input_file,
        objcopy, input_file, output_obj),
      inputs = {input_file},
      outputs = {output_obj},
    }
    local symbol_base = string.gsub(input_file, "[^a-zA-Z0-9]", "_")
    local symbol_begin = string.format("_binary_%s_start", symbol_base)
    local symbol_end = string.format("_binary_%s_end", symbol_base)
    return symbol_begin, symbol_end
  end

  function lua_sources_to_cxx(lua_sources, output_cxx)
    local command = {
      string.format("^ lua_sources.sh %s^", target),
      "bash src/dort/lua_sources.sh",
      ">"..output_cxx}
    for _, lua_source in ipairs(lua_sources) do
      local source_id = lua_source[1]
      local sym_begin = lua_source[2]
      local sym_end = lua_source[3]
      command[#command + 1] = string.format("%q %s %s", source_id, sym_begin, sym_end)
    end
    tup.definerule {
      command = table.concat(command, " "),
      inputs = {},
      outputs = {output_cxx},
    }
  end

  local lua_sources = {}
  for _, subdir in ipairs(lua_subdirs) do
    local glob = string.format("%s/%s/*.lua", lua_dir, subdir)
    for _, lua_file in ipairs(tup.glob(glob)) do
      local base = tup.base(lua_file)
      local luac_file = string.format("%s/luac/%s/%s.luac", build_dir, subdir, base)
      local obj_file = string.format("%s/luao/%s/%s.o", build_dir, subdir, base)
      local id = string.format("%s/%s", subdir, base)
      if subdir == "" then
        id = base
      end

      compile_lua(lua_file, luac_file)
      local sym_begin, sym_end = binary_file_to_obj(luac_file, obj_file)
      lua_sources[#lua_sources + 1] = {id, sym_begin, sym_end}
      dort_objs[#dort_objs + 1] = obj_file
    end
  end

  local lua_sources_cxx = string.format("%s/lua_sources.cpp", build_dir)
  local lua_sources_o = string.format("%s/o/lua_sources.o", build_dir)
  lua_sources_to_cxx(lua_sources, lua_sources_cxx)
  compile("c++", lua_sources_cxx, lua_sources_o)
  dort_objs[#dort_objs + 1] = lua_sources_o

  link(dort_objs, string.format("%s/dort", build_dir))
end

compile_target("debug")
if tup.getconfig("SLOW") != "" then
  compile_target("slow")
end
if tup.getconfig("FAST") != "" then
  compile_target("fast")
end
