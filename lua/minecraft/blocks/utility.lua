local _ENV = require "minecraft/blocks/_env"
return function(world) 
  local B = world.builder

  local torch_material = m.make_matte {
    color = rgbh("cb9e50"),
  }

  local torch_up = b.frame(B, function()
    b.set_material(B, torch_material)
    b.set_transform(B, g.stretch(g.point(1/2-1/16, 0, 1/2-1/16), g.point(1/2+1/16, 2/3, 1/2+1/16)))
    b.add_shape(B, s.make_cube())
  end)

  function torch_wall(angle)
    return b.frame(B, function()
      b.set_material(B, torch_material)
      b.set_transform(B, g.rotate_y_around(angle, g.point(1/2, 0, 1/2)))
      b.set_transform(B, g.rotate_z_around(-pi / 8, g.point(0, 1/4, 0)))
      b.set_transform(B, g.stretch(g.point(-1/16, 1/8, 1/2-1/16), g.point(1/16, 2/3, 1/2+1/16)))
      b.add_shape(B, s.make_cube())
    end)
  end

  local torches = {
    world:define_primitive_voxel(torch_wall(0)),
    world:define_primitive_voxel(torch_wall(pi)),
    world:define_primitive_voxel(torch_wall(-pi / 2)),
    world:define_primitive_voxel(torch_wall(pi / 2)),
    world:define_primitive_voxel(torch_up),
  }

  world:define_block("torch", 50, function(block)
    return torches[block.data]
  end)
end
