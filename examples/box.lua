local _ENV = require "dort/dsl"

local s = 1e-1

local scene = define_scene(function()
  local white = matte_material { color = rgb(0.5, 0.5, 0.5) }
  local green = matte_material { color = rgb(0, 0.5, 0) }
  local red = matte_material { color = rgb(0.5, 0, 0) }

  local right_box = mirror_material { color = rgb(1, 1, 1) }
  local left_box = glass_material { color = rgb(1, 1, 1) }
  right_box = white
  left_box = white

  block(function()
    local m = mesh {
      points = {
        point(552.8*s, 0, 0),
        point(0, 0, 0),
        point(0, 0, 559.2*s),
        point(549.6*s, 0, 559.2*s),

        point(556*s, 548.8*s, 0),
        point(556*s, 548.8*s, 559.2*s),
        point(0, 548.8*s, 559.2*s),
        point(0, 548.8*s, 0),
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
        translate(185*s, 82.5*s, 169*s)
      * rotate_y(-0.29) 
      * scale(165*s / 2))
    add_shape(cube())
  end)

  block(function()
    material(right_box)
    transform(
        translate(368*s, 165*s, 351*s) 
      * rotate_y(-1.27) 
      * scale(165*s / 2, 330*s / 2, 165*s / 2))
    add_shape(cube())
  end)

  camera(pinhole_camera {
    transform = look_at(
      point(278*s, 273*s, -800*s),
      point(278*s, 273*s, 0),
      vector(0, 1, 0)) * scale(1, -1, 1),
    fov = 0.686,
  })

  ---[[
  block(function() 
    transform(translate(0, -1, 0))
    local m = mesh {
      points = {
        point(343*s, 548.8*s, 227*s),
        point(343*s, 548.8*s, 332*s),
        point(213*s, 548.8*s, 332*s),
        point(213*s, 548.8*s, 227*s),
      },
      vertices = {
        2, 1, 0,  3, 2, 0,
      },
    }
    for _, index in ipairs({0, 3}) do
      add_light(diffuse_light {
        radiance = rgb(1, 1, 1) * 100,
        shape = triangle(m, index),
        num_samples = 8,
      })
    end
  end)
  --]]

  --[[
  add_light(point_light {
    point = point(250*s, 300*s, 250*s),
    intensity = rgb(1) * 3e5 *s*s,
  })
  --]]
end)

write_png_image("box_lt.png", render(scene, {
  x_res = 256, y_res = 256,
  max_depth = 10,
  sampler = stratified_sampler {
    samples_per_x = 1,
    samples_per_y = 1,
  },
  filter = mitchell_filter { radius = 2 },
  renderer = "lt",
  iterations = 20,
  --[[
  renderer = "sppm",
  iterations = 20,
  initial_radius = 10*s,
  light_paths = 256*256,
  --]]
}))
