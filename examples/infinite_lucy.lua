local quality = true

local scene = define_scene(function()
  block(function()
    material(plastic_material {
      color = rgb(1, 1, 1),
      roughness = 0.4,
    })
    transform(rotate_x(-pi / 2))
    add_read_ply_mesh_as_bvh('data/lucy.ply')
  end)

  add_light(infinite_light {
    radiance = rgb(26, 129, 224) / 256,
    num_samples = 32,
  })

  camera(perspective_camera {
    transform = look_at(
      point(1000, 2000, -3000),
      point(500, 600, 250),
      vector(0, 1, 0)),
    fov = pi * 0.2,
    screen_x = 0.9, screen_y = 0.9,
  })
end)

if quality then
  x_res = 1920
  y_res = 1080
  samples = 4
else
  x_res = 192
  y_res = 108
  samples = 1
end

write_png_image("infinite_lucy.png", render(scene, {
  sampler = stratified_sampler {
    samples_per_x = samples,
    samples_per_y = samples,
  },
  filter = mitchell_filter { radius = 1.5 },
  x_res = x_res, y_res = y_res,
}))

