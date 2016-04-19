local _ENV = require "dort/dsl"

local scene = define_scene(function()

  block(function()
    material(matte_material {
      color = rgb(1, 0, 0)
    })
    transform(translate(-1.5, 0, 0))
    add_shape(cube())
  end)

  block(function()
    material(matte_material {
      color = rgb(0, 1, 1)
    })
    transform(translate(1.5, 0, 0))
    --add_shape(sphere { radius = 1 })
  end)

  add_light(infinite_light {
    radiance = rgb(1,1,1) * 0.5,
  })

  camera(perspective_camera {
    transform = look_at(
      point(5, 3, 4),
      point(0, 0, 0),
      vector(0, 1, 0)),
    fov = pi / 4
  })
end)

write_png_image("cubes.png", render(scene, {
  x_res = 400,
  y_res = 400,
  sampler = stratified_sampler {
    samples_per_x = 4,
    samples_per_y = 4,
  },
  filter = mitchell_filter {
    radius = 1.5,
  }
}))
