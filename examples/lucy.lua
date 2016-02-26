local quality = false

local scene = define_scene(function()
  option("bvh split method", "sah")
  option("bvh leaf size", 12)

  block(function() 
    material(matte_material {
      reflect = rgb(1, 1, 1),
    })
    transform(rotate_x(-pi / 2))
    add_read_ply_mesh_as_bvh("data/lucy.ply")
  end)

  local look = point(700, 200, 250)
  local num_lights = 3
  local num_samples = 1
  if quality then num_samples = 16 end

  for i = 1, num_lights do
    local angle = i / num_lights * pi
    local r = 1200
    local eye = look + vector(cos(angle) * r, 400, -sin(angle) * r)
    block(function()
      transform(look_at(eye, look, vector(0, 1, 0)))
      add_light(diffuse_light {
        shape = disk { radius = 150 },
        radiance = rgb(1, 1, 1) * 20,
        num_samples = num_samples,
      })
    end)
  end

  camera(perspective_camera {
    transform = look_at(point(1000, 2000, -3000), look, vector(0, 1, 0)),
    fov = 0.2 * pi,
    screen_x = 1.5, screen_y = 1.5,
  })
end)

local sampler, filter, size
if quality then
  sampler = stratified_sampler { samples_per_x = 4, samples_per_y = 4 }
  filter = mitchell_filter { radius = 2 }
  size = 1600
else
  sampler = random_sampler { samples_per_pixel = 1 }
  filter = box_filter { radius = 0.5 }
  size = 800
end

write_png_image("lucy.png", render(scene, {
  x_res = size, y_res = size,
  sampler = sampler,
  filter = filter,
}))
