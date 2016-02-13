local zoom = 6
local num_lights = 3
local num_samples = 64

local scene = define_scene(function()
  local ply = read_ply_mesh("data/xyzrgb_statuette.ply")

  block(function() 
    material(plastic_material {
      reflect = rgb(1, 1, 1),
      diffuse = rgb(1, 1, 0.8),
      roughness = 0.3,
      eta = 1.5,
    })
    transform(
      scale(zoom) *
      translate(20, 0, 0) *
      rotate_x(1.05 * pi) *
      rotate_y(0.3 * pi))
    add_ply_mesh(ply)
  end)

  for i = 1, num_lights do
    local angle = pi * (i - 0.5) / num_lights - pi / 2
    local r = 200 * zoom
    block(function()
      transform(rotate_x(0.2 * pi) * rotate_y(angle) * translate(0, -150 * zoom, 0))
      add_light(diffuse_light {
        shape = disk { z = -r, radius = 80 * zoom },
        radiance = rgb(1, 1, 1) * 6 / num_lights,
        num_samples = num_samples,
      })
    end)
  end
end)

write_png_image("statue.png", render(scene, {
  x_res = 240 * zoom, y_res = 480 * zoom,
}))
