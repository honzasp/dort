local _ENV = require "dort/dsl"
local tungsten = require "tungsten"

local scene = tungsten.read("data/material-testball")
write_rgbe_image("comp/tungsten/testball.hdr", render(scene, {
  x_res = 600, y_res = 338, hdr = true,
  renderer = "pt",
  iterations = 2,
}))
