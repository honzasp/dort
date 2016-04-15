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

local stone_voxels = {}
for i = 1, #stone_colors do
  stone_voxels[i] = b.voxel(m.make_matte {
    color = stone_colors[i],
    sigma = 0.5,
  })
end

b.define("stone", 1, function(b)
  return stone_voxels[b.data + 1]
end)
b.define("cobblestone", 4, m.make_matte {
  color = rgb(0.5),
})
b.define("bedrock", 7, m.make_matte {
  color = rgb(0.2),
})
