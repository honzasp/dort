require "minecraft"
local _ENV = require "dort/dsl"

local box = boxi(vec3i(-20, 0, -20), vec3i(20, 30, 20))

local scene = define_scene(function()
  minecraft.add_world(get_builder(), {
    map = os.getenv("HOME") .. "/.minecraft/saves/Water",
    box = box,
  })

  if true then
  add_light(infinite_light {
    radiance = rgb(1,1,1) * 1,
    num_samples = 4,
  })
  else
  add_light(point_light {
    point = point(0.5, 10, 0.5),
    intensity = rgb(1,1,1) * 100,
  })
  end

  camera(perspective_camera {
    transform = look_at(
        point(-10.2, 8, -10),
        point(0.0, 4.0, 0.5),
        vector(0, 1, 0)) *
      scale(-1, 1, 1),
    fov = pi / 10,
  })
end)

local samples = 1
local res = 400

write_png_image("water.png", render(scene, {
  x_res = res,
  y_res = res,
  sampler = stratified_sampler {
    samples_per_x = samples,
    samples_per_y = samples,
  },
  filter = mitchell_filter {
    radius = 1.5,
  },
  renderer = "direct",
}))
