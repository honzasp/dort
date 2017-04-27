local geom_kinds = {"sphe", "disk", "cube", "disk2"}
local surface_kinds = {"diff", "glos", "mirr", "diel", "thdi"}
local light_kinds = {"poin_fwd", "poin_blw", "area_fwd", "dire_blw", "mixd_2", "mixd_3"}
local camera_kinds = {"pin"}

local function simple_scene(geom_kind, surface_kind, light_kind, camera_kind)
  local _ENV = require "dort/dsl"
  return define_scene(function()
    if surface_kind == "diff" then
      material(lambert_material { albedo = rgb(0.5) })
    elseif surface_kind == "glos" then
      material(phong_material { albedo = rgb(0.5), exponent = 50 })
    elseif surface_kind == "mirr" then
      material(mirror_material { albedo = rgb(1) })
    elseif surface_kind == "diel" then
      material(dielectric_material { ior_inside = 1.5 })
    elseif surface_kind == "thdi" then
      material(dielectric_material { ior_inside = 1.5, is_thin = true })
    else
      error(surface_kind)
    end

    if geom_kind == "sphe" then
      add_shape(sphere { radius = 2 })
    elseif geom_kind == "disk" then
      add_shape(disk { radius = 2 })
    elseif geom_kind == "cube" then
      block(function()
        transform(rotate_y(pi/4))
        transform(rotate_x(pi/6))
        add_shape(cube())
      end)
    elseif geom_kind == "disk2" then
      block(function()
        transform(translate(-2, 0, 0))
        transform(rotate_y(pi/2))
        transform(rotate_x(pi/10))
        add_shape(disk { radius = 2 })
      end)
      block(function()
        transform(translate(2, 0, 0))
        transform(rotate_y(pi/2))
        transform(rotate_x(-pi/10))
        add_shape(disk { radius = 2 })
      end)
    else
      error(geom_kind)
    end

    if light_kind == "poin_fwd" then
      add_light(point_light {
        point = point(0, 0, -3),
        intensity = rgb(10),
      })
    elseif light_kind == "poin_blw" then
      add_light(point_light {
        point = point(0, -3, -3),
        intensity = rgb(10),
      })
    elseif light_kind == "area_fwd" then
      block(function()
        transform(look_at(
          point(0, 0, -6),
          point(0, 0, 0),
          vector(0, 1, 0)))
        local shape = disk { radius = 1 }
        local light = diffuse_light {
          shape = shape,
          radiance = rgb(10),
        }
        add_light(light)
        add_shape(shape, light)
      end)
    elseif light_kind == "dire_blw" then
      add_light(directional_light {
        direction = vector(0, 1, 1),
        radiance = rgb(1),
      })
    elseif light_kind == "mixd_2" then
      add_light(point_light {
        point = point(0, -3, -3),
        intensity = rgb(10),
      })

      block(function()
        transform(look_at(
          point(0, 0, -6),
          point(0, 0, 0),
          vector(0, 1, 0)))
        local shape = disk { radius = 1 }
        local light = diffuse_light {
          shape = shape,
          radiance = rgb(3),
        }
        add_light(light)
        add_shape(shape, light)
      end)
    elseif light_kind == "mixd_3" then
      add_light(point_light {
        point = point(0, -3, -3),
        intensity = rgb(5),
      })

      block(function()
        transform(look_at(
          point(0, 0, -6),
          point(0, 0, 0),
          vector(0, 1, 0)))
        local shape = disk { radius = 1 }
        local light = diffuse_light {
          shape = shape,
          radiance = rgb(3),
        }
        add_light(light)
        add_shape(shape, light)
      end)

      add_light(directional_light {
        direction = vector(0, 1, 1),
        radiance = rgb(5),
      })
    else
      error(light_kind)
    end

    if camera_kind == "pin" then
      camera(pinhole_camera {
        transform = look_at(
          point(0, 0, -3),
          point(0, 0, 0),
          vector(0, 1, 0)),
        fov = pi/2,
      })
    else
      error(camera_kind)
    end
  end)
end

local base_opts = {
  min_depth = 1,
  max_depth = 2,
  x_res = 512, y_res = 512,
  sampler = dort.sampler.make_random { samples_per_pixel = 1 },
  filter = dort.filter.make_box { radius = 0.5 },
}

local render_optss = {
  dort.std.merge(base_opts, {
    renderer = "pt",
    iterations = 6,
  }),
  dort.std.merge(base_opts, {
    renderer = "lt",
    iterations = 15,
  }),
  dort.std.merge(base_opts, {
    renderer = "bdpt",
    iterations = 6,
    --debug_image_dir = "test/_bdpt_debug",
  }),
  dort.std.merge(base_opts, {
    renderer = "vcm",
    iterations = 4,
    initial_radius = 0.02
    --debug_image_dir = "test/_vcm_debug",
  }),
}

local ref_opts = dort.std.merge(base_opts, {
  renderer = "pt",
  iterations = 100,
  sample_all_lights = true,
})

return function(t)
  for _, geom_kind in ipairs(geom_kinds) do
    for _, surface_kind in ipairs(surface_kinds) do
      for _, light_kind in ipairs(light_kinds) do
        for _, camera_kind in ipairs(camera_kinds) do
          local is_delta_surf = surface_kind == "mirr" or
            surface_kind == "diel" or surface_kind == "thdi"
          local is_delta_light = light_kind == "poin_fwd" or 
            light_kind == "poin_blw" or light_kind == "dire_blw" or
            light_kind == "mixd_2" or light_kind == "mixd_3"
          if is_delta_surf and is_delta_light then goto next_iter end
          if is_delta_surf and geom_kind == "disk2" then goto next_iter end

          local test_renders = {}
          for _, render_opts in ipairs(render_optss) do
            if render_opts.renderer == "lt" and is_delta_surf then
              goto skip_render
            end

            local opts = dort.std.clone(render_opts)
            if surface_kind == "diel" and opts.renderer == "bdpt" then
              opts.iterations = 3 * opts.iterations
            end

            local vari = 4
            if surface_kind == "diel" and light_kind == "area_fwd" then
              vari = 12
            end

            test_renders[#test_renders + 1] = {
              name = opts.renderer,
              opts = opts,
              variation = vari,
              min_tile_size = 32,
            }
            ::skip_render::
          end

          local name = string.format("simple_%s_%s_%s_%s", geom_kind,
            surface_kind, light_kind, camera_kind)
          local scene = simple_scene(geom_kind,
            surface_kind, light_kind, camera_kind)
          t:test {
            name = name,
            scene = scene,
            renders = test_renders,
            ref_opts = ref_opts,
          }
          ::next_iter::
        end
      end
    end
  end
end
