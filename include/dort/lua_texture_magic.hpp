#pragma once
#include "dort/lua_texture.hpp"

namespace dort {
  template<class Out> constexpr LuaTextureOut lua_texture_out_v();
  template<> constexpr LuaTextureOut lua_texture_out_v
    <float>() { return LuaTextureOut::Float; };
  template<> constexpr LuaTextureOut lua_texture_out_v
    <Spectrum>() { return LuaTextureOut::Spectrum; };
  template<> constexpr LuaTextureOut lua_texture_out_v
    <Vec2>() { return LuaTextureOut::Vec2; };
  template<> constexpr LuaTextureOut lua_texture_out_v
    <Vec3>() { return LuaTextureOut::Vec3; };

  template<class In> constexpr LuaTextureIn lua_texture_in_v();
  template<> constexpr LuaTextureIn lua_texture_in_v
    <const DiffGeom&>() { return LuaTextureIn::Geom; };
  template<> constexpr LuaTextureIn lua_texture_in_v
    <float>() { return LuaTextureIn::Float; };
  template<> constexpr LuaTextureIn lua_texture_in_v
    <Vec2>() { return LuaTextureIn::Vec2; };
  template<> constexpr LuaTextureIn lua_texture_in_v
    <Vec3>() { return LuaTextureIn::Vec3; };
  template<> constexpr LuaTextureIn lua_texture_in_v
    <Spectrum>() { return LuaTextureIn::Spectrum; };

  template<template<class Out, class In> class Fun,
    class Out, class In, class... Args>
  bool lua_texture_handle_out_in(LuaTextureOut out_type, LuaTextureIn in_type,
      Args&&... args)
  {
    if(out_type == lua_texture_out_v<Out>() && in_type == lua_texture_in_v<In>()) {
      Fun<Out, In>::handle(std::forward<Args>(args)...);
      return true;
    } else {
      return false;
    }
  }

  template<template<class Out, class In> class Fun, class... Args>
  void lua_texture_dispatch_out_in(LuaTextureOut out_type, LuaTextureIn in_type,
      Args&&... args)
  {
    bool handled =
      lua_texture_handle_out_in<Fun, float, const DiffGeom&>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Spectrum, const DiffGeom&>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Vec2, const DiffGeom&>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Vec3, const DiffGeom&>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Vec3, const DiffGeom&>(out_type, in_type, args...) ||

      lua_texture_handle_out_in<Fun, float, float>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Spectrum, float>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Vec2, float>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Vec3, float>(out_type, in_type, args...) ||

      lua_texture_handle_out_in<Fun, float, Vec2>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Spectrum, Vec2>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Vec2, Vec2>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Vec3, Vec2>(out_type, in_type, args...) ||

      lua_texture_handle_out_in<Fun, float, Vec3>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Spectrum, Vec3>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Vec2, Vec3>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Vec3, Vec3>(out_type, in_type, args...) ||

      lua_texture_handle_out_in<Fun, float, Spectrum>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Spectrum, Spectrum>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Vec2, Spectrum>(out_type, in_type, args...) ||
      lua_texture_handle_out_in<Fun, Vec3, Spectrum>(out_type, in_type, args...) ||

      false;
    assert(handled); (void)handled;
  }

  template<template<class Out> class Fun, class Out, class... Args>
  bool lua_texture_handle_out(LuaTextureOut out_type, Args&&... args) {
    if(out_type == lua_texture_out_v<Out>()) {
      Fun<Out>::handle(std::forward<Args>(args)...);
      return true;
    } else {
      return false;
    }
  }

  template<template<class Out> class Fun, class... Args>
  void lua_texture_dispatch_out(LuaTextureOut out_type, Args&&... args) {
    bool handled =
      lua_texture_handle_out<Fun, float>(out_type, args...) ||
      lua_texture_handle_out<Fun, Spectrum>(out_type, args...) ||
      lua_texture_handle_out<Fun, Vec2>(out_type, args...) ||
      lua_texture_handle_out<Fun, Vec3>(out_type, args...);
    assert(handled); (void)handled;
  }

  template<template<class In> class Fun, class In, class... Args>
  bool lua_texture_handle_in(LuaTextureIn in_type, Args&&... args) {
    if(in_type == lua_texture_in_v<In>()) {
      Fun<In>::handle(std::forward<Args>(args)...);
      return true;
    } else {
      return false;
    }
  }

  template<template<class In> class Fun, class... Args>
  void lua_texture_dispatch_in(LuaTextureIn in_type, Args&&... args) {
    bool handled =
      lua_texture_handle_in<Fun, float>(in_type, args...) ||
      lua_texture_handle_in<Fun, const DiffGeom&>(in_type, args...) ||
      lua_texture_handle_in<Fun, Vec2>(in_type, args...) ||
      lua_texture_handle_in<Fun, Vec3>(in_type, args...);
      lua_texture_handle_in<Fun, Spectrum>(in_type, args...);
    assert(handled); (void)handled;
  }

  template<class Out, class In>
  LuaTexture::LuaTexture(std::shared_ptr<Texture<Out, In>> tex):
    out_type(lua_texture_out_v<Out>()),
    in_type(lua_texture_in_v<In>()),
    texture(tex)
  { }

  template<class Out, class In>
  std::shared_ptr<Texture<Out, In>> LuaTexture::get() const {
    auto cast_ptr = dynamic_cast<Texture<Out, In>*>(this->texture.get());
    assert(cast_ptr != nullptr);
    return std::shared_ptr<Texture<Out, In>>(
        this->texture, cast_ptr);
  }

  template<class Out, class In>
  std::shared_ptr<Texture<Out, In>> LuaTexture::check(lua_State* l) const {
    if(this->out_type != lua_texture_out_v<Out>()) {
      luaL_error(l, "Wrong texture output type");
    } else if(this->in_type != lua_texture_in_v<In>()) {
      luaL_error(l, "Wrong texture input type");
    }
    return this->get<Out, In>();
  }
}
