require "minecraft"
local _ENV = require "dort/dsl"

local box = boxi(vec3i(-20, 0, -20), vec3i(20, 30, 20))

local scene = define_scene(function()
  minecraft.add_world(get_builder(), {
    map = os.getenv("HOME") .. "/.minecraft/saves/Flat",
    box = box,
  })

  --[[
  block(function()
    transform(translate(0, 10, 0))
    material(matte_material {
      color = rgb(1, 0, 1),
    })
    add_shape(sphere {
      radius = 0.5,
    })
  end)
  --]]

  --[[
  local points = {
    point(-50, 30, -50),
    point( 50, 30, -50),
    point(-50, 30,  50),
    point( 50, 30,  50),
  }

  for _, point in ipairs(points) do
    add_light(point_light {
      intensity = rgb(1, 1, 1) * 8e3,
      point = point,
    })
  end
  --]]
  add_light(infinite_light {
    radiance = rgbh("b1e1ff") * 0.5,
    num_samples = 2,
  })

  camera(perspective_camera {
    transform = look_at(
        point(-12, 11, 8),
        point(0, 5, 2),
        vector(0, 1, 0)) *
      scale(-1, 1, 1),
    fov = pi / 3,
  })
end)

local samples = 8
local res = 800

write_png_image("minecraft_chunk.png", render(scene, {
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
