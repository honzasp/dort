local ply_mesh = read_ply("data/dragon_vrip.ply")
local brick_image = read_image("data/brick.jpg")

local scene = define_scene(function()
  local brick_dragon = instance(function()
    transform(translate(15, 400, 0) 
      * rotate_x(1.1 * pi) 
      * rotate_y(0.1 * pi))
    material(matte_material {
      refl = image_texture {
        image = brick_image,
        map = xy_texture_map(translate_y(-200) * scale(1e-1)),
      },
      sigma = 0
    })
    add_mesh(ply_mesh)
  end)

  for i = 1, 10 do
    block(function()
      transform(rotate(2 * pi * i / 10))
      transform(translate(-100, 0, 0))
      add_primitive(brick_dragon)
    end)
  end

  block(function()
    transform(translate(200, -200, -300) 
      * rotate_x(-0.8 * pi) 
      * rotate_z(-0.3 * pi))
    material(matte_material {
      refl = checkerboard_texture {
        map = xy_texture_map(),
        check_size = 10,
        even_check = white * 0.2,
        odd_check = rgb(0.7, 0.7, 0.7),
      },
      sigma = 1,
    })
    add_shape(sphere(100))
  end)

  add_light(point_light(point(-200, 0, -800), white * 1e6))
end)

write_png_image("output.png", render(scene, {
  x_res = 800,
  y_res = 800,
  seed = 42
}))
