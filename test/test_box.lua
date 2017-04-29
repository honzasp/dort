local cornell_scale = 0.01
local function cornell_scene(geom_kind, surface_kind, light_kind, camera_kind)
  local _ENV = require "dort/dsl"
  local s = cornell_scale
  return define_scene(function()
    local white = lambert_material { albedo = rgb(0.5, 0.5, 0.5) }
    local green = lambert_material { albedo = rgb(0, 0.5, 0) }
    local red = lambert_material { albedo = rgb(0.5, 0, 0) }
    local glossy_white = phong_material { albedo = rgb(0.5), exponent = 50 }
    local glossy_red = phong_material { albedo = rgb(0.5, 0, 0), exponent = 50 }
    local mirror = mirror_material { albedo = rgb(1) }
    local glass = dielectric_material { ior_inside = 1.3 }

    local right_box = white
    local left_box = white
    local right_wall = red
    local left_wall = green
    local floor = white

    if surface_kind == "diff" then
    elseif surface_kind == "glos" then
      right_box = glossy_white
      left_box = glossy_white
      right_wall = glossy_red
    elseif surface_kind == "delt" then
      right_box = mirror
      left_box = glass
      left_wall = mirror
    end

    local add_walls = true
    if geom_kind == "box" then
    elseif geom_kind == "openbox" then
      add_walls = false
    else
      error(geom_kind)
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

      if add_walls then
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
      end
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

    if camera_kind == "pin" then
      camera(pinhole_camera {
        transform = look_at(
          point(278*s, 273*s, -800*s),
          point(278*s, 273*s, 0),
          vector(0, 1, 0)) * scale(1, -1, 1),
        fov = 0.686,
      })
    elseif camera_kind == "orth" then
      camera(ortho_camera {
        transform = look_at(
          point(250*s, 200*s, -800*s),
          point(278*s, 273*s, 0),
          vector(0, 1, 0)) * scale(1, -1, 1),
        dimension = 800*s,
      })
    elseif camera_kind == "thin" then
      camera(thin_lens_camera {
        transform = look_at(
          point(278*s, 273*s, -800*s),
          point(278*s, 273*s, 0),
          vector(0, 1, 0)) * scale(1, -1, 1),
        fov = 0.686,
        lens_radius = 50*s,
        focal_distance = 1200*s,
      })
    else
      error(camera_kind)
    end

    if light_kind == "area" then
      block(function() 
        material(lambert_material { albedo = rgb(0) })
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
    elseif light_kind == "sphe" then
      block(function()
        transform(translate(250*s, 400*s, 250*s))
        material(lambert_material { albedo = rgb(0) })
        local shape = sphere { radius = 20*s }
        local light = diffuse_light {
          radiance = rgb(100),
          shape = shape,
        }
        add_light(light)
        add_shape(shape, light)
      end)
    elseif light_kind == "poin" then
      add_light(point_light {
        point = point(250*s, 400*s, 250*s),
        intensity = rgb(1) * 15e4 *s*s,
      })
    elseif light_kind == "dire" then
      add_light(directional_light {
        direction = vector(1, -1, 5),
        radiance = rgb(10),
      })
    elseif light_kind == "infi" then
      add_light(infinite_light {
        radiance = rgb(1),
      })
    elseif light_kind == "envi" then
      add_light(environment_light {
        image = read_image("data/vogl_14.hdr", { hdr = true }),
        up = vector(0, 1, 0),
        forward = vector(1, 0, 0),
        scale = rgb(8),
      })
    elseif light_kind == "beam" then
      add_light(beam_light {
        point = point(343*s, (548.8-10)*s, 332*s),
        direction = vector(0, -1, 0),
        radiance = rgb(32e4 * s),
      })
    else
      error("bad light kind " .. light_kind)
    end
  end)
end

local cornell_scenes = {
  {"box", "diff", "area", "pin", {"pt", "lt", "bdpt", "vcm"}},
  {"box", "glos", "area", "pin", {"pt", "bdpt", "vcm"}},
  --{"box", "delt", "area", "pin", {"bdpt", "vcm"}},
  {"box", "diff", "sphe", "pin", {"pt", "bdpt", "vcm"}},
  {"box", "glos", "sphe", "pin", {"pt", "bdpt", "vcm"}},
  {"box", "delt", "sphe", "pin", {"pt", "bdpt", "vcm"}},
  {"box", "diff", "poin", "pin", {"pt", "lt", "bdpt", "vcm"}},
  {"box", "glos", "poin", "pin", {"pt", "bdpt", "vcm"}},
  --{"box", "delt", "poin", "pin", {"bdpt", "vcm"}},
  {"box", "diff", "dire", "pin", {"pt", "lt", "bdpt", "vcm"}},
  {"box", "glos", "dire", "pin", {"pt", "bdpt", "vcm"}},
  {"box", "delt", "dire", "pin", {"pt", "bdpt", "vcm"}},
  --{"box", "diff", "beam", "pin", {"bdpt", "vcm"}},
  --{"box", "glos", "beam", "pin", {"bdpt", "vcm"}},
  --{"box", "delt", "beam", "pin", {"bdpt", "vcm"}},
  {"openbox", "diff", "infi", "pin", {"pt", "bdpt", "vcm"}},
  {"openbox", "glos", "infi", "pin", {"pt", "bdpt", "vcm"}},
  {"openbox", "delt", "infi", "pin", {"pt", "bdpt", "vcm"}},
  {"openbox", "diff", "envi", "pin", {"pt", "bdpt", "vcm"}},
  {"openbox", "glos", "envi", "pin", {"pt", "bdpt", "vcm"}},
  {"openbox", "delt", "envi", "pin", {"pt", "bdpt", "vcm"}},
  {"box", "diff", "area", "orth", {"pt", "bdpt", "vcm"}},
  {"box", "diff", "area", "thin", {"pt", "lt", "bdpt", "vcm"}},
  --]]
}

local base_opts = {
  x_res = 512, y_res = 512,
  sampler = dort.sampler.make_random { samples_per_pixel = 1 },
  filter = dort.filter.make_box { radius = 0.5 },
}

local render_optss = {
  pt = dort.std.merge(base_opts, {
    renderer = "pt",
    iterations = 5,
  }),
  lt = dort.std.merge(base_opts, {
    renderer = "lt",
    iterations = 10,
  }),
  bdpt = dort.std.merge(base_opts, {
    renderer = "bdpt",
    iterations = 5,
  }),
  vcm = dort.std.merge(base_opts, {
    renderer = "vcm",
    iterations = 5,
    initial_radius = 1*cornell_scale,
  }),
}

local ref_opts = dort.std.merge(base_opts, {
  renderer = "pt",
  iterations = 500,
})

return function(t)
  for _, scene_def in ipairs(cornell_scenes) do
    local geom_kind = scene_def[1]
    local surface_kind = scene_def[2]
    local light_kind = scene_def[3]
    local camera_kind = scene_def[4]
    local scene = cornell_scene(geom_kind, surface_kind, light_kind, camera_kind)

    local min_depth = 0
    local max_depth = 3
    if light_kind == "poin" then
      min_depth = 1
    end

    local ref_iter = 1
    if surface_kind == "delt" then
      ref_iter = 8
    elseif surface_kind == "glos" then
      ref_iter = 4
    end

    local function add_test_depth(min_depth, max_depth)
      local name = string.format("%s_%s_%s_%s", geom_kind,
        surface_kind, light_kind, camera_kind)
      if min_depth == max_depth then
        name = string.format("%s_d%d", name, min_depth)
      end

      local renders = {}
      for _, renderer in ipairs(scene_def[5]) do
        renders[#renders + 1] = {
          name = renderer,
          opts = dort.std.merge(render_optss[renderer], {
            min_depth = min_depth,
            max_depth = min_depth,
          }),
        }
      end

      t:test {
        name = name,
        scene = scene,
        renders = renders,
        ref_opts = dort.std.merge(ref_opts, {
          min_depth = min_depth,
          max_depth = min_depth,
          iterations = ref_opts.iterations * ref_iter,
        }),
      }
    end

    for depth = min_depth, max_depth do
      add_test_depth(depth, depth)
    end
    add_test_depth(min_depth, max_depth)
  end
end
