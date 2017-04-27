local function test_scene(tested_material)
  local _ENV = require "dort/dsl"
  return define_scene(function()
    block(function()
      transform(rotate_x(pi/2))
      material(lambert_material { albedo = rgb(0.5) })
      add_shape(disk { radius = 5 })
    end)

    block(function()
      material(tested_material)
      transform(translate(-1.5, 1, 0))
      add_shape(sphere { radius = 1 })
    end)
    block(function()
      material(tested_material)
      transform(translate(1.5, 1, 0))
      add_shape(cube())
    end)

    material(lambert_material { albedo = rgb(0) })
    add_light(point_light {
      point = point(0, 5, -3),
      intensity = rgb(20),
    })
    block(function()
      transform(look_at(
        point(-5, 5, -3),
        point(-1.5, 1, 0),
        vector(0, 1, 0)))
      add_diffuse_light {
        shape = disk { radius = 2 },
        radiance = rgb(2),
      }
    end)
    block(function()
      transform(look_at(
        point(5, 5, -3),
        point(1.5, 1, 0),
        vector(0, 1, 0)))
      add_diffuse_light {
        shape = disk { radius = 4 },
        radiance = rgb(0.7),
      }
    end)

    camera(pinhole_camera {
      transform = look_at(
        point(0, 4, -15),
        point(0, 1, 1),
        vector(0, -1, 0)),
      fov = pi/6,
    })
  end)
end

local materials = {
  {"lamb", dort.material.make_lambert {
    albedo = dort.spectrum.rgbh(0x0366d6),
  }},
  {"orennayar_0.01", dort.material.make_oren_nayar {
    albedo = dort.spectrum.rgbh(0x2ba545),
    sigma = 0.01,
  }},
  {"orennayar_0.1", dort.material.make_oren_nayar {
    albedo = dort.spectrum.rgbh(0x2ba545),
    sigma = 0.1,
  }},
  {"orennayar_0.5", dort.material.make_oren_nayar {
    albedo = dort.spectrum.rgbh(0x2ba545),
    sigma = 0.5,
  }},
  {"mirror", dort.material.make_mirror {
    albedo = dort.spectrum.rgb(1),
  }, 10},
  {"dielectric_1.5", dort.material.make_dielectric {
    ior_inside = 1.5, ior_outside = 1,
  }, 10},
  {"dielectric_3", dort.material.make_dielectric {
    ior_inside = 3, ior_outside = 1,
  }, 10},
  {"dielectric_inv1.5", dort.material.make_dielectric {
    ior_inside = 1, ior_outside = 1.5,
  }, 10},
  {"phong_50", dort.material.make_phong {
    albedo = dort.spectrum.rgbh(0xc6830d),
    exponent = 50,
  }},
  {"phong_10", dort.material.make_phong {
    albedo = dort.spectrum.rgbh(0xc6830d),
    exponent = 10,
  }},
  {"phong_200", dort.material.make_phong {
    albedo = dort.spectrum.rgbh(0xc6830d),
    exponent = 200,
  }},
}

local base_opts = {
  min_depth = 1,
  max_depth = 2,
  iterations = 1,
  x_res = 512, y_res = 338,
  sampler = dort.sampler.make_random { samples_per_pixel = 1 },
  filter = dort.filter.make_box { radius = 0.5 },
}

local render_optss = {
  dort.std.merge(base_opts, {
    renderer = "pt",
  }),
  dort.std.merge(base_opts, {
    renderer = "lt",
    iterations = 10,
  }),
  dort.std.merge(base_opts, {
    renderer = "bdpt",
    iterations = 2,
  }),
  dort.std.merge(base_opts, {
    renderer = "vcm",
    iterations = 2,
    initial_radius = 0.02
    --debug_image_dir = "test/_vcm_debug",
  }),
}

local ref_opts = dort.std.merge(base_opts, {
  renderer = "pt",
  iterations = 512,
  sample_all_lights = true,
})

return function(t)
  for _, pair in ipairs(materials) do
    local name = string.format("bsdf_%s", pair[1])
    local scene = test_scene(pair[2])

    local test_renders = {}
    for _, render_opts in ipairs(render_optss) do
      if pair[3] then
        render_opts = dort.std.merge(render_opts, { max_depth = pair[3] })
      end
      test_renders[#test_renders + 1] = {
        name = render_opts.renderer,
        opts = render_opts,
      }
    end

    local test_ref_opts = dort.std.clone(ref_opts)
    if pair[3] then
      test_ref_opts.max_depth = pair[3]
    end

    t:test {
      name = name,
      scene = scene,
      renders = test_renders,
      ref_opts = test_ref_opts,
    }
  end
end
