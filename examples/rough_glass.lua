local scene = define_scene(function()
  block(function()
    material(rough_glass_material {
      color = rgb(1,1,1) * 1,
      transmit_color = rgb(1,1,1) * 1,
      eta = 1.3,
      roughness = 0.05,
    })
    material(glass_material {
      color = rgb(1,1,1) * 1,
      transmit_color = rgb(1,1,1) * 1,
      eta = 1.2,
    })
    material(mirror_material {})
    add_shape(sphere { radius = 100 })
  end)

  block(function()
    transform(translate(10, -5, 0))
    material(matte_material {
      color = checkerboard_texture {
        map = xy_texture_map(),
        check_size = 60,
        odd_check = rgb(1,1,1) * 0.3,
        even_check = rgb(1,1,1) * 0.7,
      },
    })
    add_shape(disk { z = 180, radius = 400 })
  end)

  camera(perspective_camera {
    transform = look_at(
      point(-300, 0, -250),
      point(0, 0, 0),
      vector(0, 1, 0)),
    fov = pi / 4,
  })

  add_light(infinite_light { radiance = rgb(1,1,1) * 0.5 })
end)

write_png_image("rough_glass.png", render(scene, {
  x_res = 512, y_res = 512,
  max_depth = 10,
  sampler = stratified_sampler {
    samples_per_x = 4,
    samples_per_y = 4,
  },
  filter = mitchell_filter {
    radius = 1.5,
  },
}))

