local _ENV = require "dort/dsl"

local scene = define_scene(function()
  block(function()
    material(glass_material {
      color = rgb(1, 0.8, 0.8),
      transmit_color = rgb(0.8, 1, 0.8),
      eta = 1.8,
    })
    add_shape(sphere { radius = 1 })
  end)

  block(function()
    material(matte_material { color = rgb(0.2, 1, 1.0), })
    transform(scale(0.2))
    add_shape(cube())
  end)

  block(function()
    material(matte_material { color = rgb(1.0, 0.9, 0.3), })
    local m = mesh {
      points = {
        point(-2.5, -1.5, -3),
        point( 2.5, -1.5, -3),
        point( 2.5, -1.5,  3),
        point(-2.5, -1.5,  3),
      },
      vertices = {
        0, 1, 2,
        2, 3, 0,
      },
    }
    add_triangle(m, 0)
    add_triangle(m, 3)
  end)

  add_light(infinite_light {
    radiance = rgb(1,1,1) * 0.7,
    num_samples = 4,
  })

  camera(perspective_camera {
    transform = look_at(
        point(3, 4, -5),
        point(0, 0, 0),
        --point(0, -1.5, 2.5),
        vector(0, 1, 0)),
    fov = pi / 9,
  })
end)

local samples = 10 
local res = 100

write_png_image("droplet.png", render(scene, {
  x_res = res,
  y_res = res,
  sampler = stratified_sampler {
    samples_per_x = samples,
    samples_per_y = samples,
  },
  filter = mitchell_filter {
    radius = 1.5,
  },
  renderer = "path",
  max_depth = 10,
}))

