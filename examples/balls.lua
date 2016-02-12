local scene = define_scene(function()
  for i = 1, 10 do
    local angle = i / 10 * 2 * pi
    local r = 250
    block(function()
      material(matte_material {
        reflect = rgb(i / 10, 1 - i / 10, 0.5),
        sigma = 1,
      })
      transform(translate(cos(angle) * r, sin(angle) * r, 0))
      add_shape(sphere { radius = 50 })
    end)
  end

  add_light(point_light {
    point = point(0, 0, -300),
    intensity = rgb(1, 1, 1) * 300000,
  })
end)

write_png_image("balls.png", render(scene, {
  x_res = 800, y_res = 800,
}))
