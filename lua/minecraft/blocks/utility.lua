local _ENV = require "minecraft/blocks/_env"
return function(world) 
  do
    b.push_frame(world.builder)
    b.set_material(world.builder, m.make_matte {
      color = rgb(1, 0, 1),
    })
    b.set_transform(world.builder, g.translate(0.5, 0.5, 0.5))
    b.add_shape(world.builder, s.make_sphere {
      radius = 0.5
    })
    local torch_prim = b.pop_frame(world.builder)
    world:define_block("torch", 50, world:define_primitive_voxel(torch_prim))
  end
end
