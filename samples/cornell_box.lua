-- Renders the Cornell box to cornell_box.png. You can play with rendering
-- parameters at the bottom of the file
local _ENV = require "dort/dsl"

local scene = define_scene(function()
  local white = lambert_material { albedo = rgb(0.5, 0.5, 0.5) }
  local green = lambert_material { albedo = rgb(0, 0.5, 0) }
  local red = lambert_material { albedo = rgb(0.5, 0, 0) }

  local mirror = mirror_material { albedo = rgb(1, 1, 1) }
  local glass = dielectric_material { }
  local glossy = phong_material { albedo = rgb(0.5), exponent = 50 }

  -- Materials of the two cubes can be changed here
  local right_box = glossy
  local left_box = glossy

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
    material(left_box)
    transform(
        translate(185, 82.5, 169)
      * rotate_y(-0.29) 
      * scale(165 / 2))
    add_shape(cube())
  end)

  block(function()
    material(right_box)
    transform(
        translate(368, 165, 351) 
      * rotate_y(-1.27) 
      * scale(165 / 2, 330 / 2, 165 / 2))
    add_shape(cube())
  end)

  camera(pinhole_camera {
    transform = look_at(
      point(278, 273, -800),
      point(278, 273, 0),
      vector(0, 1, 0)) * scale(1, -1, 1),
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
      add_diffuse_light {
        radiance = rgb(1, 1, 1) * 100,
        shape = triangle(m, index),
      }
    end
  end)
end)

write_png_image("cornell_box.png", tonemap_srgb(render(scene, {
  hdr = true,
  x_res = 512, y_res = 512,
  max_depth = 3,
  renderer = "vcm",
  initial_radius = 5,
  iterations = 10,
})))
