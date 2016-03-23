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
  --[[color = t.make_lerp {
    tex_0 = rgb(1,1,1) * 0.2,
    tex_1 = rgb(1,1,1) * 0.8,
    t = t.make_noise {
      layers = {
        { scale = 37.2, weight = 2 },
        { scale = 14.3, weight = 2 },
        { scale = 8.34, weight = 1 },
        { scale = 2.21, weight = 1 },
      },
    },
  },
  --]]
  color = rgb(1,1,1) * 0.5,
})
