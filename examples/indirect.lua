require "minecraft"
local _ENV = require "dort/dsl"

local box = boxi(vec3i(-7, 0, -6), vec3i(6, 8, 6))

local scene = define_scene(function()
  minecraft.add_world(get_builder(), {
    map = os.getenv("HOME") .. "/.minecraft/saves/Indirect",
    box = box,
  })

  add_light(infinite_light {
    radiance = rgb(1,1,1) * 0.2,
    num_samples = 4,
  })

  camera(perspective_camera {
    transform = look_at(
        point(2, 6, 7),
        point(-1.2, 5, -0.7),
        vector(0, 1, 0)) *
      scale(-1, 1, 1),
    fov = pi / 3,
  })
end)

local samples = 4
local res = 400
local sets = 128
local paths = 1024

write_png_image("indirect.png", render(scene, {
  x_res = res,
  y_res = floor(res * 9 / 16),
  sampler = stratified_sampler {
    samples_per_x = samples,
    samples_per_y = samples,
  },
  filter = mitchell_filter {
    radius = 1.5,
  },
  renderer = "igi",
  light_sets = sets,
  light_paths = paths,
}))
