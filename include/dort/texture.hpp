#pragma once
#include "dort/dort.hpp"
#include "dort/vec_2.hpp"
#include "dort/vec_3.hpp"

namespace dort {
  class AnyTexture {
    virtual void dummy_to_make_this_class_polymorphic() { };
  };

  template<class Out, class In>
  class Texture: public AnyTexture {
  public:
    virtual Out evaluate(In param) const = 0;
  };

  template<class Out, class In, class F>
  class FunctionTexture final: public Texture<Out, In> {
    F function;
  public:
    FunctionTexture(F function): function(std::move(function)) { }
    virtual Out evaluate(In input) const override final {
      return this->function(input);
    }
  };

  template<class Out, class In, class F>
  std::shared_ptr<Texture<Out, In>> make_texture(F function) {
    return std::make_shared<FunctionTexture<Out, In, F>>(std::move(function));
  }

  template<class T2, class T1, class T0, class F>
  std::shared_ptr<Texture<T2, T0>> map_texture(F function,
      std::shared_ptr<Texture<T1, T0>> texture)
  {
    return make_texture<T2, T0>([=](T0 x0) {
      return function(texture->evaluate(x0));
    });
  }

  template<class Tout, class T1out, class T2out, class Tin, class F>
  std::shared_ptr<Texture<Tout, Tin>> map_texture_2(F function,
      std::shared_ptr<Texture<T1out, Tin>> tex_1,
      std::shared_ptr<Texture<T2out, Tin>> tex_2)
  {
    return make_texture<Tout, Tin>([=](Tin x0) {
      return function(tex_1->evaluate(x0), tex_2->evaluate(x0));
    });
  }

  template<class T2, class T1, class T0>
  std::shared_ptr<Texture<T2, T0>> compose_texture(
      std::shared_ptr<Texture<T2, T1>> tex_2,
      std::shared_ptr<Texture<T1, T0>> tex_1)
  {
    return make_texture<T1, T0>([=](T0 x0) {
      return tex_2->evaluate(tex_1->evaluate(x0));
    });
  }

  template<class Out, class In = const DiffGeom&>
  std::shared_ptr<Texture<Out, In>> const_texture(Out value) {
    return make_texture<Out, In>([=](In) { return value; });
  }

  template<class Out, class In>
  std::shared_ptr<Texture<Out, In>> add_texture(
      std::shared_ptr<Texture<Out, In>> tex_1,
      std::shared_ptr<Texture<Out, In>> tex_2)
  {
    return make_texture<Out, In>([=](In x) {
      return tex_1->evaluate(x) + tex_2->evaluate(x);
    });
  }

  template<class Out, class In>
  std::shared_ptr<Texture<Out, In>> scale_texture(
      std::shared_ptr<Texture<float, In>> tex_1,
      std::shared_ptr<Texture<Out, In>> tex_2)
  {
    return make_texture<Out, In>([=](In x) {
      return tex_1->evaluate(x) * tex_2->evaluate(x);
    });
  }

  template<class Out, class In>
  std::shared_ptr<Texture<Out, In>> mul_texture(
      std::shared_ptr<Texture<Out, In>> tex_1,
      std::shared_ptr<Texture<Out, In>> tex_2)
  {
    return make_texture<Out, In>([=](In x) {
      return tex_1->evaluate(x) * tex_2->evaluate(x);
    });
  }

}
