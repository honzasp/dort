local ply_dragon = read_ply_mesh("data/dragon_vrip.ply")
local ply_cube = read_ply_mesh("data/open_cube.ply")

local scene = define_scene(function()
  block(function()
    material(matte_material {
      reflect = rgb(1, 1, 1),
    })
    transform(translate(0, 0, -300) * scale(500))
    add_ply_mesh(ply_cube)
  end)

  block(function()
    material(plastic_material {
      reflect = rgb(0.9, 0.9, 0.2),
      diffuse = rgb(0.8, 0.8, 0.4),
    })
    transform(scale(3e3) * translate(0, 0.1, 0) * rotate_x(pi))
    add_ply_mesh(ply_dragon)
  end)

  add_light(diffuse_light {
    shape = disk { z = -400, radius = 100 },
    radiance = rgb(1, 1, 1) * 15,
    num_samples = 16,
  })
end)

write_png_image("diffuse_light.png", render(scene, {
  x_res = 800, y_res = 800,
}))
