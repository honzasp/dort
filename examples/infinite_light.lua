local scene = define_scene(function()
  block(function()
    material(plastic_material {
      color = rgb(1, 1, 1),
      roughness = 0.4,
    })
    transform(scale(1e3) * translate(0, -0.1, 0))
    add_read_ply_mesh('data/dragon_vrip.ply')
  end)

  add_light(infinite_light {
    radiance = rgb(26, 129, 224) / 256,
    num_samples = 32,
  })

  camera(perspective_camera {
    transform = look_at(
      point(-200, 200, -500),
      point(0, 10, 0),
      vector(0, 1, 0)),
    fov = pi / 6,
  })
end)

write_png_image("infinite_light.png", render(scene, {
  sampler = stratified_sampler {
    samples_per_x = 4,
    samples_per_y = 4,
  },
  filter = mitchell_filter { radius = 1.5 },
  x_res = 1366, y_res = 768,
}))

