local _ENV = require "dort/dsl"
local tungsten = require "tungsten"

local scene = tungsten.read("data/material-testball", {
  bsdfs = {
    --[[
    Material = dort.material.make_bump {
      bump = dort.texture.make_const_geom(0.05) * 
        (dort.texture.make_average()
        .. dort.texture.make_image(dort.image.read(
            "data/material-testball/textures/bump.png", { hdr = false }))
        .. dort.texture.make_map_uv()),
      material = dort.material.make_mirror { color = dort.spectrum.rgb(1) },
    },
    --]]
    --Material = dort.material.make_mirror { albedo = dort.spectrum.rgb(1) },
    --[[
    Material = dort.material.make_dielectric {
      ior_inside = 1.5, ior_outside = 1,
      is_thin = false,
    },--]]
    --[[
    Material = dort.material.make_oren_nayar {
      albedo = dort.spectrum.rgb(0.5),
      sigma = 0.1,
    },--]]
    Material = dort.material.make_rough_dielectric {
      reflect_tint = dort.spectrum.rgb(1),
      transmit_tint = dort.spectrum.rgb(0),
      ior_inside = 1.5, ior_outside = 1,
      roughness = 0.1,
      distribution = "beckmann",
    },
  }
})

write_rgbe_image("comp/tungsten/testball.hdr", render(scene, {
  x_res = 600, y_res = 338, hdr = true,
  renderer = "pt",
  iterations = 10,
  min_depth = 1, max_depth = 8,
}))
