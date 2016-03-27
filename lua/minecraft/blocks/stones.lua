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

local stone_noise = 
  t.make_gain(0.7) ..
  t.make_value_noise_3d {
    { weight = 6, frequency = 1/4 },
    { weight = 2, frequency = 1/2 },
    { weight = 1, frequency = 1 },
    { weight = 1, frequency = 2 },
    { weight = 1, frequency = 4 },
    { weight = 3, frequency = 8 },
    { weight = 4, frequency = 16 },
    { weight = 3, frequency = 32 },
  } ..
  t.make_identity_3d() + t.make_const_3d(1.5) * t.make_value_noise_3d_of_3d {
    { weight = 1, frequency = 1 },
    { weight = 1, frequency = 2 },
    { weight = 1, frequency = 4 },
    { weight = 2, frequency = 8 },
    { weight = 2, frequency = 16 },
  } ..
  t.make_map_xyz(g.identity())

local stone_voxels = {}
for i = 1, #stone_colors do
  stone_voxels[i] = b.voxel(m.make_matte {
    color = t.make_color_map_lerp(stone_colors[i], rgb(0.1)) .. stone_noise,
    sigma = 0.5,
  })
end

b.define("stone", 1, function(pos, id, data)
  return stone_voxels[data + 1]
end)
b.define("cobblestone", 4, m.make_matte {
  color = t.make_color_map_lerp(rgb(0.5), rgb(0.8)) .. stone_noise,
})
b.define("bedrock", 7, m.make_matte {
  color = t.make_color_map_lerp(rgb(0.1), rgb(0.4)) .. stone_noise,
})
