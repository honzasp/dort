require "minecraft"
local _ENV = require "dort/dsl"

local box = boxi(vec3i(-20, 0, -20), vec3i(20, 30, 20))

local scene = define_scene(function()
  minecraft.add_world(get_builder(), {
    map = os.getenv("HOME") .. "/.minecraft/saves/Trees",
    box = box,
  })

  add_light(infinite_light {
    radiance = rgb(1,1,1) * 1,
    num_samples = 1,
  })

  camera(perspective_camera {
    transform = look_at(
        point(-5, 7, 8),
        point(0.5, 6, 0.5),
        vector(0, 1, 0)) *
      scale(-1, 1, 1),
    fov = pi / 3,
  })
end)

local samples = 5
local res = 800

write_png_image("trees.png", render(scene, {
  x_res = res,
  y_res = res,
  sampler = stratified_sampler {
    samples_per_x = samples,
    samples_per_y = samples,
  },
  filter = mitchell_filter {
    radius = 1.5,
  },
}))
