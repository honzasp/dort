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
  --[[
  color = t.make_map_xyz(t.make_grayscale(t.compose(
    t.make_value_noise_3d {
      { weight = 1, frequency = 32 },
      { weight = 1, frequency = 16 },
      { weight = 1, frequency = 8 },
      { weight = 1, frequency = 4 },
      { weight = 1, frequency = 2 },
      { weight = 1, frequency = 1 },
      { weight = 1, frequency = 0.5 },
      { weight = 1, frequency = 0.25 },
      { weight = 1, frequency = 0.125 },
    },
    t.make_identity_3d() + t.make_const_3d(0.8) * t.make_value_noise_3d_of_3d {
      { weight = 1, frequency = 4 },
      { weight = 3, frequency = 2 },
      { weight = 1, frequency = 1},
      { weight = 4, frequency = 0.5 },
      { weight = 6, frequency = 0.25 },
    }
  ))),
  --]]
  color = t.make_map_xyz(t.make_grayscale(t.compose(
    t.make_value_noise_3d {
      { weight = 6, frequency = 1/4 },
      { weight = 2, frequency = 1/2 },
      { weight = 1, frequency = 1 },
      { weight = 1, frequency = 2 },
      { weight = 1, frequency = 4 },
      { weight = 3, frequency = 8 },
      { weight = 4, frequency = 16 },
      { weight = 3, frequency = 32 },
    },
    t.make_identity_3d() + t.make_const_3d(1.5) * t.make_value_noise_3d_of_3d {
      { weight = 1, frequency = 1 },
      { weight = 1, frequency = 2 },
      { weight = 1, frequency = 4 },
      { weight = 2, frequency = 8 },
      { weight = 2, frequency = 16 },
    }
  )))
})
