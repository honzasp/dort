local _ENV = require "minecraft/blocks/_env"

local torch_prim
dort.builder.define_scene(function()
  torch_prim = dort.builder.define_instance(function()
    dort.builder.set_material(m.make_matte {
      color = rgb(1, 0, 1),
    })
    dort.builder.set_transform(g.translate(0.5, 0.5, 0.5))
    dort.builder.add_shape(dort.shape.make_sphere {
      radius = 0.5
    })
  end)

  dort.builder.set_camera(dort.camera.make_perspective { transform = g.identity() })
end)

b.define_primitive("torch", 50, torch_prim)
