local _ENV = require "dort/dsl"
local tungsten = require "tungsten"

local scene = tungsten.read("data/veach-bidir")
write_rgbe_image("comp/tungsten/veach_bidir_vcm.hdr", render(scene, {
  x_res = 512, y_res = 512, hdr = true,
  renderer = "vcm",
  iterations = 10,
  min_depth = 0,
  max_depth = 5,
  initial_radius = 0.1,
}))
