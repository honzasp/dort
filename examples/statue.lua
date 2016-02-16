local zoom = 6
local num_lights = 3
local num_samples = 64

zoom = 1
num_samples = 1
num_lights = 8

local scene = define_scene(function()
  local ply = read_ply_mesh("data/xyzrgb_statuette.ply")
  --local ply = read_ply_mesh("data/cube.ply")

  block(function() 
    material(plastic_material {
      reflect = rgb(1, 1, 1),
      diffuse = rgb(1, 1, 0.8),
      roughness = 1.0,
      eta = 1.5,
    })
    add_ply_mesh(ply)
  end)

  for i = 1, num_lights do
    local angle = 2 * pi * (i - 0.5) / num_lights
    local r = 500
    block(function()
      translate(cos(angle) * r, -100, sin(angle) * r)
      add_light(diffuse_light {
        shape = sphere { radius = 50 },
        radiance = rgb(1, 1, 1) * 5,
      })
    end)
  end

  local camera_angle = 0.7 * pi
  block(function()
    transform(look_at(
      point(700 * sin(camera_angle), 400, 700 * -cos(camera_angle)),
      point(0, 0, 0),
      vector(0, 1, 0)))
    camera(perspective_camera {
      transform = identity(),
      fov = 0.2 * pi,
      z_near = 1e2,
      z_far = 1e5,
    })
  end)
end)

write_png_image("statue.png", render(scene, {
  x_res = 240 * zoom, y_res = 480 * zoom,
}))
