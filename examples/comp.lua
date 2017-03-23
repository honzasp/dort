local _ENV = require "dort/dsl"

function cornell_box_scene(surface_kind, light_kind)
  local s = 0.1
  return define_scene(function()
    local white = matte_material { color = rgb(0.5, 0.5, 0.5) }
    local green = matte_material { color = rgb(0, 0.5, 0) }
    local red = matte_material { color = rgb(0.5, 0, 0) }
    local glossy_white = phong_material { color = rgb(0.5), exponent = 50 }
    local glossy_red = phong_material { color = rgb(0.5, 0, 0), exponent = 50 }
    local mirror = mirror_material { color = rgb(1) }
    local glass = glass_material { color = rgb(1) }

    local right_box = white
    local left_box = white
    local right_wall = red
    local left_wall = green
    local floor = white

    if surface_kind == "diffuse" then
    elseif surface_kind == "glossy" then
      right_box = glossy_white
      left_box = glossy_white
      right_wall = glossy_red
    elseif surface_kind == "delta" then
      right_box = mirror
      left_box = glass
      left_wall = mirror
    end

    block(function()
      local m = mesh {
        points = {
          point(552.8*s, 0, 0),
          point(0, 0, 0),
          point(0, 0, 559.2*s),
          point(549.6*s, 0, 559.2*s),

          point(556*s, 548.8*s, 0),
          point(556*s, 548.8*s, 559.2*s),
          point(0, 548.8*s, 559.2*s),
          point(0, 548.8*s, 0),
        },
        vertices = {
          0, 1, 2,  0, 2, 3,
          4, 5, 6,  4, 6, 7,
          6, 5, 3,  6, 3, 2,
          7, 6, 2,  7, 2, 1,
          0, 3, 5,  0, 5, 4,
        },
      }

      material(floor)
      add_triangle(m, 0)
      add_triangle(m, 3)
      add_triangle(m, 6)
      add_triangle(m, 9)
      add_triangle(m, 12)
      add_triangle(m, 15)

      material(left_wall)
      add_triangle(m, 18)
      add_triangle(m, 21)

      material(right_wall)
      add_triangle(m, 24)
      add_triangle(m, 27)
    end)

    block(function()
      material(left_box)
      transform(
          translate(185*s, 82.5*s, 169*s)
        * rotate_y(-0.29) 
        * scale(165*s / 2))
      add_shape(cube())
    end)

    block(function()
      material(right_box)
      transform(
          translate(368*s, 165*s, 351*s) 
        * rotate_y(-1.27) 
        * scale(165*s / 2, 330*s / 2, 165*s / 2))
      add_shape(cube())
    end)

    camera(pinhole_camera {
      transform = look_at(
        point(278*s, 273*s, -800*s),
        point(278*s, 273*s, 0),
        vector(0, 1, 0)) * scale(1, -1, 1),
      fov = 0.686,
    })

    if light_kind == "area" then
      block(function() 
        material(matte_material { color = rgb(0) })
        transform(translate(0, -1*s, 0))
        local m = mesh {
          points = {
            point(343*s, 548.8*s, 227*s),
            point(343*s, 548.8*s, 332*s),
            point(213*s, 548.8*s, 332*s),
            point(213*s, 548.8*s, 227*s),
          },
          vertices = {
            2, 1, 0,  3, 2, 0,
          },
        }
        for _, index in ipairs({0, 3}) do
          local shape = triangle(m, index)
          local light = diffuse_light {
            radiance = rgb(100),
            shape = shape,
          }
          add_light(light)
          add_shape(shape, light)
        end
      end)
    elseif light_kind == "sphere" then
      block(function()
        transform(translate(250*s, 400*s, 250*s))
        material(matte_material { color = rgb(0) })
        local shape = sphere { radius = 20*s }
        local light = diffuse_light {
          radiance = rgb(100),
          shape = shape,
        }
        add_light(light)
        add_shape(shape, light)
      end)
    elseif light_kind == "point" then
      add_light(point_light {
        point = point(250*s, 400*s, 250*s),
        intensity = rgb(1) * 15e4 *s*s,
      })
    else
      error("bad light kind " .. light_kind)
    end
  end)
end

function cavern_scene()
  local minecraft = require "minecraft"
  return define_scene(function()
    block(function()
      minecraft.add_world(get_builder(), {
        map = os.getenv("HOME") .. "/.minecraft/saves/Cavern",
        box = boxi(vec3i(-30, 80, -30), vec3i(30, 100, 30)),
      })
    end)

    local lights = {
      {point(-4.2, 94.3, -4.8), 150},
      {point(-12.0, 94.1, -5.5), 300},
      {point(-6.3, 96.7, 7.1), 150},
      {point(-1.1, 98.2, 11.9), 300},
      {point(-5.2, 92.9, -2.2), 150},
    }

    for _, light_pair in ipairs(lights) do
      --[[
      add_light(point_light {
        point = light_pair[1],
        intensity = rgb(light_pair[2]),
      })
      --]]
      add_light(diffuse_light {
        shape = sphere { radius = 0.3 },
        radiance = rgb(light_pair[2] * 3),
        transform = translate(light_pair[1] - point(0,0,0)),
      })
    end

    camera(pinhole_camera {
      transform = look_at(
        point(3.5, 100.4, -0.1),
        point(10.7, 92.3, -10.4),
        vector(0, 1, 0)) * scale(-1, -1, 1),
      fov = pi / 3,
    })
  end)
end

function light_disk_scene()
  local s = 1
  return define_scene(function()
    block(function()
      material(matte_material { color = rgb(0.5) })
      add_shape(disk { radius = 5*s, z = 10*s })
    end)

    block(function()
      material(matte_material { color = rgb(0) })
      local shape = sphere { radius = 1*s }
      transform(translate(4*s, 0*s, 6*s))
      local light = diffuse_light {
        shape = shape,
        radiance = rgb(1),
        transform = identity(),
      }
      add_shape(shape, light)
      add_light(light)
    end)

    camera(pinhole_camera {
      transform = look_at(
        point(0, 0, 0),
        point(0, 0, 10*s),
        vector(0, 1, 0)),
      fov = pi / 2,
    })
  end)
end

function sphere_light_scene()
  local a = 1
  local b = 1
  return define_scene(function()
    block(function()
      material(matte_material { color = rgb(0.5) })
      add_shape(disk { radius = 0.2*b, z = 0 })
    end)

    block(function()
      transform(translate(3*a, 0, -4*a))
      add_light(diffuse_light {
        radiance = rgb(1*a*a),
        shape = sphere { radius = 1*a },
      })
    end)

    camera(pinhole_camera {
      transform = look_at(
        point(0, 0, -10*b),
        point(0, 0, 0),
        vector(0, 1, 0)),
      fov = pi / 2,
    })
  end)
end

function point_light_scene()
  local a = 10
  local b = 1
  return define_scene(function()
    block(function()
      material(matte_material { color = rgb(0.5) })
      add_shape(disk { radius = 0.2*b, z = 0 })
    end)

    add_light(point_light {
      point = point(3*a, 0, -4*a),
      intensity = rgb(100*a*a),
    })

    camera(pinhole_camera {
      transform = look_at(
        point(0, 0, -10*b),
        point(0, 0, 0),
        vector(0, 1, 0)),
      fov = pi / 2,
    })
  end)
end

out_dir = "comp"

local res = 512
local x_res = res
local y_res = res
common_opts = {
  x_res = x_res, y_res = y_res,
  hdr = true,
  sampler = random_sampler {
    samples_per_pixel = 1
  },
  filter = box_filter { radius = 0.5 },
}

algos = {
  --[[
  {"dot", {
    renderer = "dot",
  }},
  --]]
  {"direct", {
    max_depth = 1,
    renderer = "direct",
    iterations = 4,
  }},
  --]]
  ---[[
  {"lt", {
    min_depth = 0,
    max_depth = 4,
    renderer = "lt",
    iterations = 10,
  }},
  --]]
  ---[[
  {"pt", {
    min_depth = 0,
    max_depth = 4,
    renderer = "pt",
    iterations = 10,
  }},
  --]]
  --[[
  {"lt_0", {renderer = "lt", min_depth = 0, max_depth = 0, iterations = 2}},
  {"lt_1", {renderer = "lt", min_depth = 1, max_depth = 1, iterations = 2}},
  {"lt_2", {renderer = "lt", min_depth = 2, max_depth = 2, iterations = 2}},
  {"lt_3", {renderer = "lt", min_depth = 3, max_depth = 3, iterations = 2}},
  {"pt_0", {renderer = "pt", min_depth = 0, max_depth = 0, iterations = 2}},
  {"pt_1", {renderer = "pt", min_depth = 1, max_depth = 1, iterations = 2}},
  {"pt_2", {renderer = "pt", min_depth = 2, max_depth = 2, iterations = 2}},
  {"pt_3", {renderer = "pt", min_depth = 3, max_depth = 3, iterations = 2}},
  --]]
  ---[[
  {"bdpt", {
    min_depth = 0,
    max_depth = 4,
    renderer = "bdpt",
    iterations = 10,
    debug_image_dir = out_dir .. "/_bdpt_debug",
  }},
  --[[
  {"ref", {
    max_depth = 5,
    renderer = "sppm",
    iterations = 2000,
    initial_radius = 0.5,
    light_paths = res*res,
  }},
  --]]
}
scenes = {
  --{"point_light", point_light_scene()},
  --{"sphere_light", sphere_light_scene()},
  --{"light_disk", light_disk_scene()},
  --{"diffuse_box", cornell_box_scene("diffuse", "area")},
  --{"diffuses_box", cornell_box_scene("diffuse", "sphere")},
  {"glossy_box", cornell_box_scene("glossy", "area")},
  --{"delta_box", cornell_box_scene("delta", "area")},
  --{"diffusep_box", cornell_box_scene("diffuse", "point")},
  --{"glossyp_box", cornell_box_scene("glossy", "point")},
  --{"deltap_box", cornell_box_scene("delta", "point")},
  --[[
  --]]
  --{"cavern", cavern_scene()},
}

for scene_i = 1, #scenes do
  for algo_i = 1, #algos do
    local algo_name = algos[algo_i][1]
    local scene_name = scenes[scene_i][1]

    opts = {}
    for key, value in pairs(common_opts) do
      opts[key] = value
    end
    for key, value in pairs(algos[algo_i][2]) do
      opts[key] = value
    end

    os.execute("mkdir -p " .. out_dir .. "/" .. scene_name)
    image_name = scene_name.."/"..algo_name
    print(image_name)

    hdr_image = render(scenes[scene_i][2], opts)
    write_rgbe_image(out_dir.."/"..image_name..".hdr", hdr_image)
    --srgb_image = tonemap_srgb(hdr_image)
    --write_png_image(out_dir.."/"..image_name..".png", srgb_image)
  end
end
