local _ENV = require "dort/dsl"

for angle = 0, 180, 10 do
  local scene = define_scene(function()
    material(bump_material {
      bump = 
        (const_texture_3d(1) * value_noise_texture_3d {
          { weight = 1, frequency = 1 },
        }) .. 
        (identity_texture_3d() + const_texture_3d(1) * value_noise_texture_3d_of_3d {
          { weight = 1, frequency = 2 },
        }) ..
        xyz_texture_map(translate(1.234, 2.345, 3.456)),
      material = plastic_material {
        color = rgb(0.5),
        roughness = 0.5,
      },
    })

    add_shape(sphere {
      radius = 20
    })

    local rad = angle / 180 * pi
    add_light(point_light {
      point = point(cos(rad) * 80, 0, sin(rad) * 80),
      intensity = rgb(1) * 1e4,
    })

    camera(perspective_camera {
      transform = look_at(
        point(0, 0, 30),
        point(0, 0, 0),
        vector(0, 1, 0)),
    })
  end)

  write_png_image("bump_" .. angle .. ".png", render(scene, {
    x_res = 400, y_res = 400,
    sampler = stratified_sampler {
      samples_per_x = 1,
      samples_per_y = 1,
    },
  }))
end
