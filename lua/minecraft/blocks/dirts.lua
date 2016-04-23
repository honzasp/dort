local _ENV = require "minecraft/blocks/_env"
return function(world)
  world:define_block("grass", 2, world:define_cube_voxel {
    bottom = m.make_matte {
      color = rgb(152, 188, 91) / 256,
      sigma = 0.2
    },
    side = m.make_matte {
      color = rgb(134, 96, 67) / 256,
    },
  })

  world:define_block("dirt", 3, world:define_cube_voxel(
    m.make_matte { color = rgb(134, 96, 67) / 256 }
  ))

  world:define_block("sand", 12, world:define_cube_voxel(
    m.make_matte { color = rgb(219, 211, 160) / 256 }
  ))

  world:define_block("gravel", 13, world:define_cube_voxel(
    m.make_matte { color = rgb(127, 124, 123) / 256 }
  ))
end
