local scene = define_scene(function()
  block(function()
    material(plastic_material {
      color = rgb(0.9, 0.9, 0.1),
      reflect_color = rgb(1, 1, 1),
      roughness = 1.5,
    })
    transform(translate(0, -50, 0) * scale(1e3))
    add_read_ply_mesh_as_bvh("data/dragon_vrip.ply")
  end)

  block(function()
    transform(rotate_x(pi / 2) * scale(10))
    material(matte_material {
      color = checkerboard_texture {
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
      color = rgb(1, 0.2, 0.2),
      reflect_color = rgb(1, 1, 1),
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
      color = rgb(1, 0, 0),
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

filters = {
  box_1 = box_filter { radius = 1 },
  box_3 = box_filter { radius = 3 },
  triangle_2 = triangle_filter { radius = 2 },
  gauss_1_1 = gaussian_filter { radius = 1, alpha = 1 },
  gauss_1_3 = gaussian_filter { radius = 1, alpha = 3 },
  gauss_3_1 = gaussian_filter { radius = 3, alpha = 1 },
  gauss_3_3 = gaussian_filter { radius = 3, alpha = 3 },
  mitchell_1 = mitchell_filter { radius = 1 },
  mitchell_2 = mitchell_filter { radius = 2 },
  mitchell_3 = mitchell_filter { radius = 3 },
  mitchell_5 = mitchell_filter { radius = 5 },
  lanczos_2 = lanczos_sinc_filter { radius = 2 },
  lanczos_3 = lanczos_sinc_filter { radius = 3 },
  lanczos_5 = lanczos_sinc_filter { radius = 5 },
  lanczos = lanczos_sinc_filter { radius = 3, tau = 1.5 },
}

for filter in pairs(filters) do
  write_png_image("filter_" .. filter .. ".png", render(scene, {
    x_res = 800, y_res = 800,
    filter = filters[filter],
  }))
end

