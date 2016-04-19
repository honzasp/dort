local _ENV = require "minecraft/blocks/_env"
return function(world) 
  do
    local B = world.builder
    b.push_frame(B)
    b.set_material(B, m.make_matte {
      color = rgb(1, 0, 1),
    })
    b.set_transform(B, g.stretch(g.point(0.4, 0, 0.4), g.point(0.6, 0.66, 0.6)))
    b.add_shape(B, s.make_cube())
    local torch_prim = b.pop_frame(B)
    world:define_block("torch", 50, world:define_primitive_voxel(torch_prim))
  end
end
