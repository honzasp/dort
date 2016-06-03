-- This scene definition is a straightforward transliteration of the scenes
-- defined in SmallVCM (http://www.smallvcm.com) by Tomas Davidovic
-- (http://www.davidovic.cz).
local _ENV = require "dort/dsl"

function box_scene(params)
  return define_scene(function()
    local light_mat = matte_material { color = rgb(0) }
    local glossy_white_mat = phong_material {
      color = rgb(0.1),
      glossy_color = rgb(0.7),
      exponent = 90,
    }
    local diffuse_green_mat = matte_material {
      color = rgb(0.156863, 0.803922, 0.172549),
    }
    local diffuse_red_mat = matte_material {
      color = rgb(0.803922, 0.152941, 0.152941),
    }
    local diffuse_blue_mat = matte_material {
      color = rgb(0.156863, 0.172549, 0.803922),
    }
    local diffuse_white_mat = matte_material {
      color = rgb(0.803922, 0.803922, 0.803922),
    }
    local mirror_mat = mirror_material { color = rgb(1) }
    local glass_mat = glass_material { color = rgb(1), eta = 1.6 }

    block(function()
      local pos = point(-0.0439815, -4.12529, 0.222539)
      local forward = vector(0.00688625, 0.998505, -0.0542161)
      local up = vector(3.73896e-4, 0.0542148, 0.998529)
      camera(perspective_camera {
        transform = look_at(pos, pos + forward, up) * scale(-1, 1, 1),
        fov = pi / 4,
      })
    end)

    local light_box = not params.light_point

    local cb_points = {
      point(-1.27029,  1.30455, -1.28002),
      point( 1.28975,  1.30455, -1.28002),
      point( 1.28975,  1.30455,  1.28002),
      point(-1.27029,  1.30455,  1.28002),
      point(-1.27029, -1.25549, -1.28002),
      point( 1.28975, -1.25549, -1.28002),
      point( 1.28975, -1.25549,  1.28002),
      point(-1.27029, -1.25549,  1.28002),
    }

    local cb = mesh {
      points = cb_points,
      vertices = {
        0, 4, 5,  5, 1, 0,
        0, 1, 2,  2, 3, 0,
        2, 6, 7,  7, 3, 2,
        3, 7, 4,  4, 0, 3,
        1, 5, 6,  6, 2, 1,
      }
    }

    -- floor
    if params.glossy_floor then
      material(glossy_white_mat)
    else
      material(diffuse_white_mat)
    end
    add_triangle(cb, 0)
    add_triangle(cb, 3)

    --back wall
    if params.glossy_floor then
      material(diffuse_blue_mat)
    else
      material(diffuse_white_mat)
    end
    add_triangle(cb, 6)
    add_triangle(cb, 9)

    -- ceiling
    if params.light_ceiling and not light_box then
      material(light_mat)
      add_diffuse_light {
        shape = triangle(cb, 12),
        radiance = rgb(0.95492965),
      }
      add_diffuse_light {
        shape = triangle(cb, 15),
        radiance = rgb(0.95492965),
      }
    else
      material(diffuse_white_mat)
      add_triangle(cb, 12);
      add_triangle(cb, 15);
    end

    -- left wall
    material(diffuse_green_mat)
    add_triangle(cb, 18)
    add_triangle(cb, 21)

    -- right wall
    material(diffuse_red_mat)
    add_triangle(cb, 24)
    add_triangle(cb, 27)

    -- central ball
    block(function()
      local large_radius = 0.8
      local center = (cb_points[1] + cb_points[2] + cb_points[5] + cb_points[6]) 
        * 0.25 + vector(0, 0, large_radius)
      transform(translate(vector(center)))
      if params.large_mirror_sphere then
        material(mirror_mat)
        add_shape(sphere { radius = large_radius })
      elseif params.large_glass_sphere then
        material(glass_mat)
        add_shape(sphere { radius = large_radius })
      end
    end)

    -- left and right balls
    block(function()
      local small_radius = 0.5
      local left_wall_center = (cb_points[1] + cb_points[5]) * 0.5 + vector(0, 0, small_radius)
      local right_wall_center = (cb_points[2] + cb_points[6]) * 0.5 + vector(0, 0, small_radius)
      local x_len = right_wall_center:x() - left_wall_center:x()
      local left_ball_center = left_wall_center + vector(2 * x_len / 7, 0, 0)
      local right_ball_center = right_wall_center - vector(2 * x_len / 7, 0, 0)

      if params.small_mirror_sphere then
        block(function()
          transform(translate(vector(left_ball_center)))
          material(mirror_mat)
          add_shape(sphere { radius = small_radius })
        end)
      end

      if params.small_glass_sphere then
        block(function()
          transform(translate(vector(right_ball_center)))
          material(glass_mat)
          add_shape(sphere { radius = small_radius })
        end)
      end
    end)

    -- light box
    local lb_points = {
      point(-0.25,  0.25, 1.26002),
      point( 0.25,  0.25, 1.26002),
      point( 0.25,  0.25, 1.28002),
      point(-0.25,  0.25, 1.28002),
      point(-0.25, -0.25, 1.26002),
      point( 0.25, -0.25, 1.26002),
      point( 0.25, -0.25, 1.28002),
      point(-0.25, -0.25, 1.28002),
    }
    local lb = mesh {
      points = lb_points,
      vertices = {
        0, 2, 1,  2, 0, 3,
        3, 4, 7,  4, 3, 0,
        1, 6, 5,  6, 1, 2,
        4, 5, 6,  6, 7, 4,
        0, 4, 5,  5, 1, 0,
      }
    }

    if light_box then
      material(diffuse_white_mat)
      add_triangle(lb, 0)
      add_triangle(lb, 3)
      add_triangle(lb, 6)
      add_triangle(lb, 9)
      add_triangle(lb, 12)
      add_triangle(lb, 15)
      add_triangle(lb, 18)
      add_triangle(lb, 21)

      if params.light_ceiling then
        material(light_mat)
        add_diffuse_light {
          shape = triangle(lb, 24),
          radiance = rgb(25.03329895614464),
        }
        add_diffuse_light {
          shape = triangle(lb, 27),
          radiance = rgb(25.03329895614464),
        }
      else
        add_triangle(lb, 24)
        add_triangle(lb, 27)
      end
    end

    if params.light_sun then
      add_light(directional_light {
        radiance = rgb(0.5, 0.2, 0) * 20,
        direction = vector(-1, 1.5, -1),
      })
    end

    if params.light_point then
      add_light(point_light {
        point = point(0, -0.5, 1),
        intensity = rgb(70 / (4 * pi)),
      })
    end

    if params.light_background then
      add_light(infinite_light {
        radiance = rgb(135, 206, 250) / 255,
      })
    end
  end)
end

local scene_ggbs_s = box_scene {
  glossy_floor = true, 
  small_mirror_sphere = true,
  small_glass_sphere = true,
  light_sun = true,
}

local scene_gglm_d = box_scene {
  glossy_floor = true,
  large_mirror_sphere = true,
  light_ceiling = true,
}

local scene_ggbs_p = box_scene {
  glossy_floor = true,
  small_mirror_sphere = true,
  small_glass_sphere = true,
  light_point = true,
}

local scene_ggbs_b = box_scene {
  glossy_floor = true,
  small_mirror_sphere = true,
  small_glass_sphere = true,
  light_background = true,
}


local res = 512
local samples = 1
local filter = mitchell_filter { radius = 1.5 }
local sampler = stratified_sampler {
  samples_per_x = samples, samples_per_y = samples,
}

function render_direct(scene, scene_name)
  write_png_image("box_direct_" .. scene_name .. ".png", render(scene, {
    x_res = res, y_res = res,
    sampler = sampler,
    filter = filter,
    renderer = "direct",
  }))
end

function render_igi(scene, scene_name)
  write_png_image("box_igi_" .. scene_name .. ".png", render(scene, {
    x_res = res, y_res = res,
    sampler = sampler,
    filter = filter,
    renderer = "igi",
    light_sets = samples * samples,
    light_paths = 512,
  }))
end

function render_sppm(scene, scene_name)
  write_rgbe_image("box_sppm_" .. scene_name .. ".hdr", render(scene, {
    x_res = res, y_res = res,
    sampler = sampler,
    filter = filter,
    renderer = "sppm",
    light_paths = 100*1000,
    iterations = 8,
    initial_radius = 0.1,
    alpha = 0.5,
    max_light_depth = 10,
    max_depth = 10,
    hdr = true,
    parallel_mode = "serial_iterations",
  }))
end


function render_pt(scene, scene_name)
  write_png_image("box_pt_" .. scene_name .. ".png", render(scene, {
    x_res = res, y_res = res,
    sampler = stratified_sampler {
      samples_per_x = 3 * samples, samples_per_y = 3 * samples,
    },
    filter = filter,
    renderer = "pt",
  }))
end

--[[
render_igi(scene_ggbs_s, "ggbs_s")
render_igi(scene_ggbs_p, "ggbs_p")
render_igi(scene_ggbs_b, "ggbs_b")
render_igi(scene_gglm_d, "gglm_c")
--]]

--[[
render_direct(scene_ggbs_s, "ggbs_s")
render_direct(scene_ggbs_p, "ggbs_p")
render_direct(scene_ggbs_b, "ggbs_b")
render_direct(scene_gglm_d, "gglm_c")
--]]

--[[
render_pt(scene_ggbs_s, "ggbs_s")
render_pt(scene_ggbs_p, "ggbs_p")
render_pt(scene_ggbs_b, "ggbs_b")
render_pt(scene_gglm_d, "gglm_c")
--]]

--render_sppm(scene_ggbs_s, "ggbs_s")
render_sppm(scene_ggbs_p, "ggbs_p")
--render_sppm(scene_ggbs_b, "ggbs_b")
--render_sppm(scene_gglm_d, "gglm_c")
