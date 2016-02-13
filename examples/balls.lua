local scene = define_scene(function()
  local cube = read_ply_mesh("data/cube.ply")
  for j = 1, 5 do
    z = (j - 2.5) * 100
    for i = 1, 10 do
      local angle = (i + j) / 10 * 2 * pi
      local r = 80
      block(function()
        material(matte_material {
          reflect = rgb(i / 10, 1 - i / 10, 0.5),
          sigma = 1,
        })
        transform(translate(cos(angle) * r, sin(angle) * r, z))
        transform(scale(20))
        add_ply_mesh(cube)
      end)
    end

    add_light(point_light {
      point = point(0, 0, z),
      intensity = rgb(1, 1, 1) * 100000,
    })
  end

  transform(translate(0, 0, 600))
  camera(perspective_camera {
    transform = identity(),
    fov = pi * 0.2,
  })
  --[[camera(ortho_camera {
    transform = identity(),
    screen_x = 800,
    screen_y = 800,
  })
  --]]
end)

write_png_image("balls.png", render(scene, {
  x_res = 800, y_res = 800,
}))
