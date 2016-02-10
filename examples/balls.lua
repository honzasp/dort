local scene = define_scene(function()
  material(matte_material {
    reflect = rgb(1, 0.9, 0.2),
  })
  add_shape(sphere { radius = 100 })
  add_light(point_light {
    point = point(-100, 200, -300),
    intensity = rgb(1, 1, 1) * 100000,
  })
end)

write_png_image("balls.png", render(scene, {
  x_res = 800, y_res = 800,
}))
