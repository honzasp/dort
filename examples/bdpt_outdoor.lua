local _ENV = require 'dort/dsl'
local minecraft = require "minecraft"

local scene = define_scene(function()
  minecraft.add_world(get_builder(), {
    map = os.getenv("HOME") .. "/.minecraft/saves/Testbed",
    box = boxi(vec3i(-20, 0, -20), vec3i(20, 20, 20)),
  })
  ---[[
  add_light(infinite_light {
    radiance = rgb(0.5),
  })
  --]]
  --[[
  for z = -20, 20, 10 do
    for x = -20, 20, 10 do
      add_light(point_light {
        point = point(x, 20, z),
        intensity = rgb(60),
      })
    end
  end
  --]]
end)

write_png_image("bdpt_outdoor.png", tonemap_srgb(render(scene, {
  hdr = true, x_res = 300, y_res = 200,
  camera = pinhole_camera {
    transform = look_at(
      point(10, 20, -50),
      point(-12, 6, -1),
      vector(0, 1, 0)
    ) * scale(-1, -1, 1),
    fov = pi / 6,
  },
  filter = mitchell_filter { radius = 1.5 },
  sampler = stratified_sampler {
    samples_per_x = 2,
    samples_per_y = 2,
  },
  renderer = "lt",
  iterations = 10,
})))
