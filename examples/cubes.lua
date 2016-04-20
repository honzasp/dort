local _ENV = require "dort/dsl"
local scene = define_scene(function()
  block(function()
    material(matte_material {
      color = rgb(1, 0, 0)
    })
    transform(stretch(point(-1, 0, 0), point(0, 1, 1)))
    add_shape(cube())
  end)

  block(function()
    material(matte_material {
      color = rgb(1, 1, 0)
    })
    transform(rotate_y_around(pi / 2, point(0.5, 0, 0.5)))
    transform(rotate_z(-pi / 8))
    transform(stretch(point(-0.1, 0, 0.4), point(0.1, 2/3, 0.6)))
    add_shape(cube())
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
    samples_per_x = 2,
    samples_per_y = 2,
  },
  filter = mitchell_filter {
    radius = 1.5,
  }
}))
