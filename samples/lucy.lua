-- IMPORTANT: to run this sample, please download the Lucy PLY model from:
-- https://graphics.stanford.edu/data/3Dscanrep/
-- and save the file "lucy.ply" into "data/lucy.ply" (relative to your working
-- directory when executing the program)
local _ENV = require "dort/dsl"

local scene = define_scene(function()
  block(function()
    material(lambert_material { albedo = rgb(0.5) })

    local x0, x1 = -2000, 4000
    local y0, y1 = -3000, 3000
    local z0, z1 = -4000, 2000

    local m = mesh {
      points = {
        point(x1, y0, z0),
        point(x0, y0, z0),
        point(x0, y0, z1),
        point(x1, y0, z1),

        point(x1, y1, z0),
        point(x1, y1, z1),
        point(x0, y1, z1),
        point(x0, y1, z0),
      },
      vertices = {
        0, 1, 2,  0, 2, 3,
        4, 5, 6,  4, 6, 7,
        6, 5, 3,  6, 3, 2,
        7, 6, 2,  7, 2, 1,
        0, 3, 5,  0, 5, 4,
      },
    }

    add_triangle(m, 0)
    add_triangle(m, 3)
    add_triangle(m, 6)
    add_triangle(m, 9)
    add_triangle(m, 12)
    add_triangle(m, 15)

    add_triangle(m, 18)
    add_triangle(m, 21)

    add_triangle(m, 24)
    add_triangle(m, 27)
  end)

  block(function()
    material(lambert_material { albedo = rgb(0.5) })
    transform(rotate_x(-pi / 2))
    transform(translate(0, -800, 0))
    add_read_ply_mesh_as_bvh('data/lucy.ply')
  end)

  camera(pinhole_camera {
    transform = look_at(
      point(1000, 2000, -3000),
      point(500, 600, 250),
      vector(0, -1, 0)),
    fov = pi * 0.2,
  })

  add_diffuse_light {
    transform = translate(500, 2000, 250),
    shape = sphere { radius = 100 },
    radiance = rgbh(0xffda36) * 300,
  }
  add_diffuse_light {
    transform = translate(500, 600, -2000),
    shape = sphere { radius = 100 },
    radiance = rgbh(0xffda36) * 200,
  }
  add_light(directional_light {
    direction = vector(0, 0, 1),
    radiance = rgb(1.2),
  })
end)

local res = 1000
write_rgbe_image("lucy.hdr", (render(scene, {
  hdr = true,
  filter = mitchell_filter { radius = 0.7 },
  x_res = res, y_res = floor(res/16*9),
  renderer = "pt",
  iterations = 20,
  max_depth = 20,
  sample_all_lights = true,
})))
