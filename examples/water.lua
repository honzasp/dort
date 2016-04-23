require "minecraft"
local _ENV = require "dort/dsl"

local box = boxi(vec3i(-20, 0, -20), vec3i(20, 30, 20))

local scene = define_scene(function()
  minecraft.add_world(get_builder(), {
    map = os.getenv("HOME") .. "/.minecraft/saves/Water",
    box = box,
  })

  add_light(infinite_light {
    radiance = rgb(1,1,1),
    num_samples = 4,
  })
  add_light(diffuse_light {
    transform = translate(0.5, 2.0, 0.5),
    shape = sphere { radius = 0.3 },
    radiance = rgb(1, 1, 1) * 30,
  })
  block(function()
    transform(translate(0.5, 2.0, 0.5))
    material(matte_material { color = rgb(1, 1, 0) })
    add_shape(sphere { radius = 0.3-0.001 })
  end)

  camera(perspective_camera {
    transform = look_at(
        point(-10.2, 8, -10),
        point(0.0, 4.0, 0.5),
        vector(0, 1, 0)) *
      scale(-1, 1, 1),
    fov = pi / 10,
  })
end)

local samples = 4
local res = 100

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
  max_depth = 10,
}))
