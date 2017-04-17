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
    Material = dort.material.make_mirror { color = dort.spectrum.rgb(1) },
  }
})

write_rgbe_image("comp/tungsten/testball.hdr", render(scene, {
  x_res = 600, y_res = 338, hdr = true,
  renderer = "pt",
  iterations = 10,
}))
