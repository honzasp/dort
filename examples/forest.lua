require "minecraft"
local _ENV = require "dort/dsl"

local box = boxi(vec3i(-100, 0, -100), vec3i(100, 150, 100))

local scene = define_scene(function()
  minecraft.add_world(get_builder(), {
    map = os.getenv("HOME") .. "/.minecraft/saves/Forest",
    box = box,
  })

  add_light(infinite_light {
    radiance = rgb(1,1,1) * 1,
    num_samples = 4,
  })

  camera(perspective_camera {
    transform = look_at(
        point(-100, 80, -100),
        point(0.5, 40, 0.5),
        vector(0, 1, 0)) *
      scale(-1, 1, 1),
    fov = pi / 3,
  })
end)

local samples = 4
local res = 1200

write_png_image("forest.png", render(scene, {
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
