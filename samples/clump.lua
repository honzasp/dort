-- Clump of random spheres and cubes
local _ENV = require "dort/dsl"
local scene = define_scene(function()
  camera(pinhole_camera {
    transform = look_at(
      point(0, 0, -30),
      point(0, 0, 0),
      vector(0, 1, 0)),
    fov = pi/6,
  })

  add_diffuse_light {
    transform = translate(-12, -8, -8),
    shape = sphere { radius = 2 },
    radiance = rgbh(0xffffff) * 50,
  }
  add_diffuse_light {
    transform = translate(8, -4, -8),
    shape = sphere { radius = 1 },
    radiance = rgbh(0xff0000) * 100,
  }
  add_diffuse_light {
    transform = translate(8, 9, -6),
    shape = sphere { radius = 1 },
    radiance = rgbh(0x0000ff) * 100,
  }
  add_light(infinite_light {
    radiance = rgbh(0x2C9CFF) * 0.8,
  })

  block(function()
    local materials = {
      lambert_material { albedo = rgb(0.5) },
      lambert_material { albedo = rgbh(0xc02b2b) },
      lambert_material { albedo = rgbh(0x42c842) },
      phong_material { albedo = rgbh(0xc3ac34), exponent = 10 },
      phong_material { albedo = rgbh(0x5555ff), exponent = 10 },
      lambert_material { albedo = rgbh(0xaa00aa) },
      lambert_material { albedo = rgbh(0x4fcccc) },
      mirror_material { },
      mirror_material { },
      dielectric_material { },
      dielectric_material { is_thin = true },
    }

    local rng = make_rng(12349)
    for i = 1, 250 do
      block(function()
        transform(translate(rng(-5, 5), rng(-5, 5), rng(-5, 5)))
        transform(scale(0.1 * pow(10, rng(0, 1))))
        material(materials[floor(rng(1, #materials))])
        if rng() < 0.8 then
          add_shape(sphere { radius = 1 })
        else
          transform(rotate_x(rng(0, 2*pi)))
          transform(rotate_y(rng(0, 2*pi)))
          transform(rotate_z(rng(0, 2*pi)))
          add_shape(cube())
        end
      end)
    end
  end)
end)

write_png_image("clump.png", tonemap_srgb(render(scene, {
  hdr = true, x_res = 800, y_res = 600,
  filter = mitchell_filter { radius = 0.7 },
  renderer = "pt",
  iterations = 30,
  max_depth = 8,
  sample_all_lights = true,
})))
