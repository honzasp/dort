require "dort.std"
local test_dir = "test"

local colors = {
  red = "\x1b[31m",
  green = "\x1b[32m",
  yellow = "\x1b[33m",
  reset = "\x1b[0m",
}

local tests = {}

function test(name, scene, render_optss)
  tests[#tests + 1] = {name, scene, render_optss}
end

function run_tests()
  local count = 0
  local errs = 0
  local time_s = 0
  for _, test in ipairs(tests) do
    local test_count, test_err, test_s = run_test(test[1], test[2], test[3])
    count = count + test_count
    errs = errs + test_err
    time_s = time_s + test_s
  end

  dort.std.printf("Tests took %g s\n", time_s)
  if errs == 0 then
    dort.std.printf("%s%d tests passed%s\n",
      colors.green, count, colors.reset)
  else
    dort.std.printf("%s%d/%d tests failed%s\n",
      colors.red, errs, count, colors.reset)
  end
end

function run_test(name, scene, render_optss)
  dort.std.printf("test %s\n", name)
  local ref_path = string.format("%s/ref/%s.hdr", test_dir, name)
  local out_dir_path = string.format("%s/out/%s", test_dir, name)

  -- TODO: nasty hack
  os.execute(string.format("mkdir -p %q", out_dir_path))
  local ref_image = dort.image.read(ref_path, { hdr = true })
  dort.image.write_rgbe(out_dir_path .. "/ref.hdr", ref_image)

  local count = 0
  local errs = 0
  local time_s = 0
  for _, render_opts in ipairs(render_optss) do
    dort.std.printf("  %s...", render_opts.renderer)
    io.stdout:flush()

    local opts_copy = dort.std.clone(render_opts)

    local render = dort.render.make(scene, opts_copy)
    local time_begin = dort.chrono.now()
    dort.render.render_sync(render)
    local time_end = dort.chrono.now()
    local out_image = dort.render.get_image(render, { hdr = true })
    dort.image.write_rgbe(string.format("%s/%s.hdr",
      out_dir_path, render_opts.renderer), out_image)

    local test_time_s = dort.chrono.difference_s(time_begin, time_end)

    local x_res, y_res = dort.image.get_res(ref_image)
    local bias, variance = dort.image.bias_variance(ref_image, out_image,
      dort.geometry.recti(0, 0, x_res, y_res))
    local norm_bias = bias / (3 * x_res * y_res)
    local norm_sd = dort.math.sqrt(variance / (3 * x_res * y_res))

    local status
    if dort.math.abs(norm_bias) < 0.05 then
      if norm_sd < 1.5 then
        status = "ok"
      else
        status = "warn"
      end
    else
      status = "err"
    end

    local color = ({
      ok = colors.green,
      warn = colors.yellow,
      err = colors.red,
    })[status]

    dort.std.printf(" %s%s (%g s): %g, %g%s\n", color, status, test_time_s,
      norm_bias, norm_sd, colors.reset)
    count = count + 1
    if status == "err" then
      errs = errs + 1
    end
    time_s = time_s + test_time_s
  end

  return count, errs, time_s
end

function cornell_scene(geom_kind, surface_kind, light_kind, camera_kind)
  local _ENV = require "dort/dsl"
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
    elseif light_kind == "sphe" then
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
        radiance = rgb(32e3),
      })
    else
      error("bad light kind " .. light_kind)
    end
  end)
end

local cornell_scenes = {
  {"box", "diff", "area", "pin", {"pt", "lt", "bdpt"}},
  {"box", "glos", "area", "pin", {"pt", "bdpt"}},
  {"box", "delt", "area", "pin", {"bdpt"}},
  {"box", "diff", "sphe", "pin", {"pt", "bdpt"}},
  {"box", "glos", "sphe", "pin", {"pt", "bdpt"}},
  {"box", "delt", "sphe", "pin", {"pt", "bdpt"}},
  {"box", "diff", "poin", "pin", {"pt_min_1", "lt_min_1", "bdpt_min_1"}},
  {"box", "glos", "poin", "pin", {"pt_min_1", "bdpt_min_1"}},
  {"box", "delt", "poin", "pin", {"bdpt_min_1"}},
  {"box", "diff", "dire", "pin", {"pt", "lt", "bdpt"}},
  {"box", "glos", "dire", "pin", {"pt", "bdpt"}},
  {"box", "delt", "dire", "pin", {"pt", "bdpt"}},
  {"box", "diff", "beam", "pin", {"bdpt"}},
  {"box", "glos", "beam", "pin", {"bdpt"}},
  {"box", "delt", "beam", "pin", {"bdpt"}},
  {"openbox", "diff", "infi", "pin", {"pt", "bdpt"}},
  {"openbox", "glos", "infi", "pin", {"pt", "bdpt"}},
  {"openbox", "delt", "infi", "pin", {"pt", "bdpt"}},
  {"openbox", "diff", "envi", "pin", {"pt", "bdpt"}},
  {"openbox", "glos", "envi", "pin", {"pt", "bdpt"}},
  {"openbox", "delt", "envi", "pin", {"pt", "bdpt"}},
  {"box", "diff", "area", "orth", {"pt", "bdpt"}},
  {"box", "diff", "area", "thin", {"pt", "lt", "bdpt"}},
}

local cornell_renderers = {
  pt = {
    renderer = "pt",
    min_depth = 0,
    max_depth = 4,
    iterations = 1,
    x_res = 512, y_res = 512,
    sampler = dort.sampler.make_random { samples_per_pixel = 1},
    filter = dort.filter.make_box { radius = 0.5 },
  },
  pt_min_1 = {
    renderer = "pt",
    min_depth = 1,
    max_depth = 4,
    iterations = 1,
    x_res = 512, y_res = 512,
    sampler = dort.sampler.make_random { samples_per_pixel = 1},
    filter = dort.filter.make_box { radius = 0.5 },
  },
  lt = {
    renderer = "lt",
    min_depth = 0,
    max_depth = 4,
    iterations = 1,
    x_res = 512, y_res = 512,
    sampler = dort.sampler.make_random { samples_per_pixel = 1},
    filter = dort.filter.make_box { radius = 0.5 },
  },
  lt_min_1 = {
    renderer = "lt",
    min_depth = 1,
    max_depth = 4,
    iterations = 1,
    x_res = 512, y_res = 512,
    sampler = dort.sampler.make_random { samples_per_pixel = 1},
    filter = dort.filter.make_box { radius = 0.5 },
  },
  bdpt = {
    renderer = "bdpt",
    min_depth = 0,
    max_depth = 4,
    iterations = 1,
    x_res = 512, y_res = 512,
    sampler = dort.sampler.make_random { samples_per_pixel = 1},
    filter = dort.filter.make_box { radius = 0.5 },
  },
  bdpt_min_1 = {
    renderer = "bdpt",
    min_depth = 1,
    max_depth = 4,
    iterations = 1,
    x_res = 512, y_res = 512,
    sampler = dort.sampler.make_random { samples_per_pixel = 1},
    filter = dort.filter.make_box { radius = 0.5 },
    debug_image_dir = "test/_bdpt_debug",
  },
}

for _, scene_def in ipairs(cornell_scenes) do
  local scene = cornell_scene(scene_def[1], scene_def[2],
    scene_def[3], scene_def[4])
  local render_optss = {}
  for _, renderer in ipairs(scene_def[5]) do
    render_optss[#render_optss + 1] = cornell_renderers[renderer]
  end
  test(string.format("%s_%s_%s_%s", scene_def[1], scene_def[2],
    scene_def[3], scene_def[4]), scene, render_optss)
end

run_tests()
