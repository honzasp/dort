-- Tetrahedron fractal
local _ENV = require "dort/dsl"

local scene = define_scene(function()
  local function generate(level)
    return frame(function()
      if level <= 0 then
        material(lambert_material {
          albedo = rgbh(0xC7570A)
        })
        add_shape(sphere { radius = 1.5 })
        return
      end

      local sublevel = generate(level - 1)
      block(function()
        transform(translate(0, 1, 1 - sqrt(2)) * scale(0.5))
        add_primitive(sublevel)
      end)
      block(function()
        transform(translate(-sqrt(3)/2, -0.5, 1 - sqrt(2)) * scale(0.5))
        add_primitive(sublevel)
      end)
      block(function()
        transform(translate(sqrt(3)/2, -0.5, 1 - sqrt(2)) * scale(0.5))
        add_primitive(sublevel)
      end)
      block(function()
        transform(translate(0, 0, 1) * scale(0.5))
        add_primitive(sublevel)
      end)
    end)
  end

  add_primitive(generate(7))
  camera(pinhole_camera {
    transform = look_at(
      point(-9, -6, 2),
      point(0, 0, 0.2),
      vector(0, 0, -1)),
    fov = pi / 8
  })

  add_diffuse_light {
    transform = translate(9, -6, -1),
    shape = sphere { radius = 0.2 },
    radiance = rgb(1e4),
  }
  add_diffuse_light {
    transform = translate(-7, 6, 1),
    shape = sphere { radius = 0.2 },
    radiance = rgb(1e4),
  }
end)

write_png_image("fractal.png", tonemap_srgb(render(scene, {
  hdr = true, y_res = 1000, x_res = 1000,
  filter = mitchell_filter { radius = 0.7 },
  renderer = "pt",
  iterations = 20,
  max_depth = 2,
  sample_all_lights = true,
})))
