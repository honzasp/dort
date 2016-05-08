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

  function torch_wall_tip(angle)
    return (g.rotate_y_around(angle, g.point(1/2, 0, 1/2)) 
      * g.rotate_z_around(-pi / 8, g.point(0, 1/4, 0)))
      :apply(g.point(0, 2/3+3/8, 1/2))
  end

  local torch_voxels = {
    world:define_primitive_voxel(torch_wall(0)),
    world:define_primitive_voxel(torch_wall(pi)),
    world:define_primitive_voxel(torch_wall(-pi / 2)),
    world:define_primitive_voxel(torch_wall(pi / 2)),
    world:define_primitive_voxel(torch_up),
  }

  local torch_tips = {
    torch_wall_tip(0),
    torch_wall_tip(pi),
    torch_wall_tip(-pi / 2),
    torch_wall_tip(pi / 2),
    g.point(1/2, 2/3+3/8, 1/2),
  }

  local torch_lightball = s.make_sphere { radius = 1/4 }

  world:define_block("torch", 50, function(block)
    local tip = g.vector(block.pos) + g.vector(torch_tips[block.data])
    b.add_light(B, l.make_diffuse {
      shape = torch_lightball,
      radiance = rgbh("f1ddbe") * 8,
      num_samples = world:option("torch_num_samples"),
      transform = g.translate(tip)
    })
    return torch_voxels[block.data]
  end)
end
