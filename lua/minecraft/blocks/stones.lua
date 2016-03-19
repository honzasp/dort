local _ENV = require "minecraft/blocks/_env"

local stone_colors = {
  rgb(125, 125, 125) / 256,
  rgb(153, 114, 99) / 256,
  rgb(153, 114, 99) / 256,
  rgb(180, 180, 180) / 256,
  rgb(180, 180, 180) / 256,
  rgb(131, 131, 131) / 256,
  rgb(131, 131, 131) / 256,
}

local stone_voxels = {}
for i = 1, #stone_colors do
  stone_voxels[i] = b.voxel(m.make_matte {
    color = stone_colors[i],
    sigma = 0.5,
  })
end

b.define("stone", 1, function(pos, id, data)
  return stone_voxels[data + 1]
end)
b.define("cobblestone", 4, m.make_matte {
  color = rgb(0.3, 0.3, 0.3)
})
b.define("bedrock", 7, m.make_matte {
  color = rgb(0.1, 0.1, 0.1)
})
