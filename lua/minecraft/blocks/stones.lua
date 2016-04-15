local _ENV = require "minecraft/blocks/_env"

local stone_colors = {
  rgbh(125, 125, 125),
  rgbh(153, 114, 99),
  rgbh(153, 114, 99),
  rgbh(180, 180, 180),
  rgbh(180, 180, 180),
  rgbh(131, 131, 131),
  rgbh(131, 131, 131),
}

return function(world)
  local stone_voxels = {}
  for i = 1, #stone_colors do
    stone_voxels[i] = world:define_cube_voxel(m.make_matte {
      color = stone_colors[i],
      sigma = 0.5,
    })
  end

  local cobble_voxel = world:define_cube_voxel(m.make_matte {
    color = rgb(0.5)
  })
  local bedrock_voxel = world:define_cube_voxel(m.make_matte {
    color = rgb(0.2)
  })

  world:define_block("stone", 1, function(b)
    return stone_voxels[b.data + 1]
  end)
  world:define_block("cobblestone", 4, cobble_voxel);
  world:define_block("bedrock", 7, bedrock_voxel);
end
