local geom_kinds = {"sphe", "disk", "cube"}
local surface_kinds = {"diff", "glos", "mirr", "diel", "thdi"}
local light_kinds = {"poin_fwd", "poin_blw"}
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
    else
      error(geom_kind)
    end

    if light_kind == "poin_fwd" then
      add_light(point_light {
        point = point(0, 0, -3),
        intensity = rgb(100),
      })
    elseif light_kind == "poin_blw" then
      add_light(point_light {
        point = point(0, -3, -3),
        intensity = rgb(100),
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
  min_depth = 0,
  max_depth = 2,
  iterations = 1,
  x_res = 512, y_res = 512,
  sampler = dort.sampler.make_random { samples_per_pixel = 1 },
  filter = dort.filter.make_box { radius = 0.5 },
}

local render_optss = {
  dort.std.merge(base_opts, {
    renderer = "pt",
  }),
  dort.std.merge(base_opts, {
    renderer = "lt",
    iterations = 2,
  }),
  dort.std.merge(base_opts, {
    renderer = "bdpt",
  }),
  dort.std.merge(base_opts, {
    renderer = "vcm",
    initial_radius = 0.1,
  }),
}

return function(t)
  for _, geom_kind in ipairs(geom_kinds) do
    for _, surface_kind in ipairs(surface_kinds) do
      for _, light_kind in ipairs(light_kinds) do
        for _, camera_kind in ipairs(camera_kinds) do
          local name = string.format("simple_%s_%s_%s_%s", geom_kind,
            surface_kind, light_kind, camera_kind)
          local scene = simple_scene(geom_kind,
            surface_kind, light_kind, camera_kind)
          t:test(name, scene, render_optss)
        end
      end
    end
  end
end
