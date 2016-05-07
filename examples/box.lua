local _ENV = require "dort/dsl"

local scene = define_scene(function()
  local white = matte_material { color = rgb(0.5, 0.5, 0.5) }
  local green = matte_material { color = rgb(0, 0.5, 0) }
  local red = matte_material { color = rgb(0.5, 0, 0) }

  block(function()
    local m = mesh {
      points = {
        point(552.8, 0, 0),
        point(0, 0, 0),
        point(0, 0, 559.2),
        point(549.6, 0, 559.2),

        point(556, 548.8, 0),
        point(556, 548.8, 559.2),
        point(0, 548.8, 559.2),
        point(0, 548.8, 0),
      },
      vertices = {
        0, 1, 2,  0, 2, 3,
        4, 5, 6,  4, 6, 7,
        6, 5, 3,  6, 3, 2,
        7, 6, 2,  7, 2, 1,
        0, 3, 5,  0, 5, 4,
      },
    }

    material(white)
    add_triangle(m, 0)
    add_triangle(m, 3)
    add_triangle(m, 6)
    add_triangle(m, 9)
    add_triangle(m, 12)
    add_triangle(m, 15)

    material(green)
    add_triangle(m, 18)
    add_triangle(m, 21)

    material(red)
    add_triangle(m, 24)
    add_triangle(m, 27)
  end)

  block(function()
    material(white)
    transform(
        translate(185, 82.5, 169)
      * rotate_y(-0.29) 
      * scale(165 / 2))
    add_shape(cube())
  end)

  block(function()
    material(white)
    transform(
        translate(368, 165, 351) 
      * rotate_y(-1.27) 
      * scale(165 / 2, 330 / 2, 165 / 2))
    add_shape(cube())
  end)

  camera(perspective_camera {
    transform = look_at(
      point(278, 273, -800),
      point(278, 273, 0),
      vector(0, 1, 0)) * scale(-1, 1, 1),
    fov = 0.686,
  })

  block(function() 
    transform(translate(0, -1, 0))
    local m = mesh {
      points = {
        point(343, 548.8, 227),
        point(343, 548.8, 332),
        point(213, 548.8, 332),
        point(213, 548.8, 227),
      },
      vertices = {
        2, 1, 0,  3, 2, 0,
      },
    }
    for _, index in ipairs({0, 3}) do
      add_light(diffuse_light {
        radiance = rgb(1, 1, 1) * 70,
        shape = triangle(m, index),
        num_samples = 8,
      })
    end
  end)
end)

write_png_image("box_igi.png", render(scene, {
  x_res = 256, y_res = 256,
  max_depth = 10,
  sampler = stratified_sampler {
    samples_per_x = 2,
    samples_per_y = 2,
  },
  filter = mitchell_filter {
    radius = 1.5,
  },
  renderer = "igi",
  light_sets = 4,
  light_paths = 32,
}))
