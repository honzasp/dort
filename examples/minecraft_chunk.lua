require "minecraft"
local _ENV = require "dort/dsl"
local quality = false

local box = boxi(vec3i(-20, 0, -20), vec3i(20, 30, 20))

local scene = define_scene(function()
  minecraft.add_world {
    map = os.getenv("HOME") .. "/.minecraft/saves/Stones",
    box = box,
  }

  if not quality then
    local points = {
      point(-50, 30, -50),
      point( 50, 30, -50),
      point(-50, 30,  50),
      point( 50, 30,  50),
    }

    for _, point in ipairs(points) do
      add_light(point_light {
        intensity = rgb(1, 1, 1) * 8e3,
        point = point,
      })
    end
  else
    add_light(infinite_light {
      radiance = rgb(1, 1, 1),
      num_samples = 16,
    })
  end

  camera(perspective_camera {
    transform = look_at(
        point(10, 5, 10),
        point(0, 0, 0),
        vector(0, 1, 0)) *
      scale(-1, 1, 1),
    fov = pi / 3,
  })
end)

local samples
if quality then
  samples = 4
else
  samples = 1
end

write_png_image("minecraft_chunk.png", render(scene, {
  x_res = 800,
  y_res = 800,
  sampler = stratified_sampler {
    samples_per_x = samples,
    samples_per_y = samples,
  },
  filter = mitchell_filter {
    radius = 1.5,
  },
}))
