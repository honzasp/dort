local scene = define_scene(function()
  local dragon = read_ply_mesh("data/dragon_vrip.ply")

  block(function()
    material(plastic_material {
      reflect = rgb(1, 1, 1),
      diffuse = rgb(0.9, 0.9, 0.1),
      roughness = 1.5,
    })
    transform(translate(0, -50, 0) * scale(1e3))
    add_ply_mesh(dragon)
  end)

  block(function()
    transform(rotate_x(pi / 2) * scale(10))
    material(matte_material {
      reflect = checkerboard_texture {
        even_check = rgb(1, 1, 1),
        odd_check = rgb(0.3, 0.3, 0.3),
        map = xy_texture_map(),
      },
    })
    add_shape(disk { z = 0, radius = 20 })
  end)
  
  block(function()
    transform(translate(-30, 50, -100))
    material(plastic_material {
      reflect = rgb(1, 1, 1),
      diffuse = rgb(1, 0.2, 0.2),
      roughness = 0.1,
    })
    add_shape(sphere { radius = 20 })
  end)

  add_light(point_light {
    point = point(0, 400, -500),
    intensity = rgb(1, 1, 1) * 500000,
  })

  block(function()
    transform(translate(0, 500, -200) * rotate_x(pi / 2))
    material(matte_material {
      reflect = rgb(1, 0, 0),
    })
    add_light(diffuse_light {
      shape = disk { z = 0, radius = 100 },
      radiance = rgb(1, 1, 1) * 8,
      num_samples = 4,
    })
  end)

  camera(perspective_camera {
    transform = look_at(
      point(100, 200, -600),
      point(0, 0, 0),
      vector(0, 1, 0)),
    fov = 0.2 * pi,
  })
end)

write_png_image("sampler.png", render(scene, {
  x_res = 800, y_res = 800,
  samples_per_pixel = 16,
  filter = mitchell_filter { radius = 2 },
}))


