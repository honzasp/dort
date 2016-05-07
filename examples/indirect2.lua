require "minecraft"
local _ENV = require "dort/dsl"

local box = boxi(vec3i(-30, 30, -10), vec3i(20, 60, 20))

local scene = define_scene(function()
  minecraft.add_world(get_builder(), {
    map = os.getenv("HOME") .. "/.minecraft/saves/Indirect2",
    box = box,
    water_glass = false,
  })

  camera(perspective_camera {
    transform = look_at(
        point(-13.5, 45.2, 13.4),
        point(-4.8, 44, 3.3),
        vector(0, 1, 0)) *
      scale(-1, 1, 1),
    fov = pi / 3,
  })
end)

local samples = 1
local res = 400
local sets = samples * samples
local paths = 800

write_png_image("indirect2.png", render(scene, {
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
  max_depth = 10,
}))
