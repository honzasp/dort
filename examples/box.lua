local scene = define_scene(function()
  local white = diffuse_material { color = rgb(0.5, 0.5, 0.5) }
  local green = diffuse_material { color = rgb(0, 0.5, 0) }
  local red = diffuse_material { color = rgb(0.5, 0, 0) }

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
        0, 1, 2,  1, 2, 3,
        4, 5, 6,  5, 6, 7,
        6, 5, 3,  5, 3, 2,
        7, 6, 2,  6, 2, 1,
        0, 3, 5,  3, 5, 4,
      },
    }

    material(white)
    add_shape(triangle(m, 0))
    add_shape(triangle(m, 3))
    add_shape(triangle(m, 6))
    add_shape(triangle(m, 9))
    add_shape(triangle(m, 12))
    add_shape(triangle(m, 15))

    material(green)
    add_shape(triangle(m, 18))
    add_shape(triangle(m, 21))

    material(red)
    add_shape(triangle(m, 24))
    add_shape(triangle(m, 27))
  end)

  block(function()
    material(white)
    transform(rotate_y(-0.29) * translate(185, 82.5, 169) * scale(165 / 2))
    add_shape(cube())
  end)

  block(function()
    material(white)
    transform(rotate_y(-1.27) * translate(368, 165, 351) * scale(165 / 2, 330 / 2, 165 / 2))
    add_shape(cube())
  end)

  camera(perspective_camera {
    transform = look_at(
      point(278, 273, -800),
      point(278, 273, 0),
      vector(0, 1, 0)),
    fov = 0.686,
  })

  block(function() 
    local m = mesh {
      points = {
        point(343, 548.8, 227),
        point(343, 548.8, 332),
        point(213, 548.8, 332),
        point(213, 548.8, 227),
      },
      vertices = {
        0, 1, 2,  2, 1, 3,
      },
    }
    add_light(diffuse_light {
      radiance = rgb(1, 1, 1) * 15,
      shape = mesh_shape(m),
    })
  end)
end)
