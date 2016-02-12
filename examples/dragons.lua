local scene = define_scene(function()
  local dragon = read_ply_mesh("data/dragon_vrip.ply")

  local dragon_prim = instance(function()
    material(plastic_material {
      reflect = rgb(1, 1, 1),
      diffuse = rgb(0.9, 0.9, 0.1),
      roughness = 1.5,
    })
    transform(rotate_x(pi))
    transform(scale(1e3))
    transform(translate(0, -0.1, 0))
    add_ply_mesh(dragon)
  end)
  for i = 1, 10 do
    block(function()
      local angle = i / 10 * 2 * pi
      transform(translate(cos(angle) * 300, sin(angle) * 300, 0))
      transform(scale(0.9))
      transform(rotate_y(i / 10 * pi))
      add_primitive(dragon_prim)
    end)
  end

  add_light(point_light {
    point = point(0, 0, -500),
    intensity = rgb(1, 1, 1) * 500000,
  })
end)

write_png_image("dragons.png", render(scene, {
  x_res = 800, y_res = 800
}))
