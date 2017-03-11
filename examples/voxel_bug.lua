require "minecraft"
local _ENV = require "dort/dsl"

function make_scene(y)
  return define_scene(function()
    block(function()
      minecraft.add_world(get_builder(), {
        map = os.getenv("HOME") .. "/.minecraft/saves/Cavern",
        box = boxi(vec3i(-30, 80, -30), vec3i(30, 100, 30)),
      })
    end)

    camera(pinhole_camera {
      transform = look_at(
        point(3.5, y, -0.1),
        point(10.7, 92.3, -10.4),
        vector(0, 1, 0)) * scale(-1, -1, 1),
      fov = pi / 3,
    })
  end)
end

for y = 95, 105 do
  local scene = make_scene(y + 0.4)
  print(y)
  write_png_image("voxel_bug_"..y..".png", render(scene, {
    x_res = 256, y_res = 256,
    filter = box_filter { radius = 0.5 },
    renderer = "dot",
  }))
end
