local quality = true
local buddha_ply = read_ply_mesh('data/happy_vrip.ply')

function test_scene(name, make_material, render_opts)
  local scene = define_scene(function()
    local white_color = rgb(0.8, 0.8, 0.8)
    local red_color = rgb(0.8, 0.4, 0.4)
    local blue_color = rgb(0.4, 0.4, 0.8)

    local white = matte_material { color = white_color }
    local white_and_blue = matte_material {
      color = checkerboard_texture {
        map = xy_texture_map(rotate_y(pi / 2)),
        check_size = 10,
        even_check = white_color,
        odd_check = blue_color,
      }
    }
    local white_and_red = matte_material {
      color = checkerboard_texture {
        map = xy_texture_map(),
        check_size = 5,
        even_check = white_color,
        odd_check = red_color,
      }
    }

    block(function()
      local walls_mesh = mesh {
        points = {
          point(-100, 0, -600),
          point(-100, 0,  100),
          point( 100, 0,  100),
          point( 100, 0, -600),
          point(-100, 200, -600),
          point(-100, 200,  100),
          point( 100, 200,  100),
          point( 100, 200, -600),
        },
        vertices = {
          0,1,2, 0,2,3, -- down (0, 3)
          4,5,6, 4,6,7, -- up (6, 9)
          1,2,6, 1,6,5, -- back (12, 15)
          2,3,7, 2,7,6, -- right (18, 21)
          0,1,5, 0,5,4, -- left (24, 27)
        },
      }

      material(white)
      add_shape(triangle(walls_mesh, 0))
      add_shape(triangle(walls_mesh, 3))
      add_shape(triangle(walls_mesh, 18))
      add_shape(triangle(walls_mesh, 21))
      add_shape(triangle(walls_mesh, 6))
      add_shape(triangle(walls_mesh, 9))

      material(white_and_blue)
      add_shape(triangle(walls_mesh, 24))
      add_shape(triangle(walls_mesh, 27))

      material(white_and_red)
      add_shape(triangle(walls_mesh, 12))
      add_shape(triangle(walls_mesh, 15))
    end)

    block(function()
      material(make_material())
      transform(translate(0, 100, 60) 
        * scale(9e2) * translate(0, -0.18, -0.1)
        * rotate_y(1.1 * pi))
      add_ply_mesh(buddha_ply)
    end)

    block(function()
      local eyes = {
        point(-99, 100, -100),
        point( 99, 100, -100),
        point(0, 150, 0),
        point(50, 100, -400),
      }

      local looks = {
        point(0, 100, -100),
        point(0, 100, -100),
        point(0, 100, 100),
        point(0, 100, -100),
      }

      local ups = {
        vector(0, 1, 0),
        vector(0, 1, 0),
        vector(0, 0, 1),
        vector(0, 1, 0),
      }

      local radiances = {
        rgb(1, 1, 1) * 15,
        rgb(1, 1, 1) * 25,
        rgb(1, 1, 1) * 20,
        rgb(1, 1, 1) * 120,
      }

      local num_samples
      if quality then num_samples = 4 else num_samples = 1 end

      for i = 1, #eyes do
        add_light(diffuse_light {
          shape = disk { radius = 20 },
          transform = look_at(eyes[i], looks[i], ups[i]),
          radiance = radiances[i],
          num_samples = num_samples,
        })
      end
    end)

    camera(perspective_camera {
      transform = look_at(
        point(0, 100, -500),
        point(5, 70, 0),
        vector(0, 1, 0)),
      fov = 2 * atan2(1, 4) + 0.02 * pi,
      screen_x = 1.4,
      screen_y = 1.4,
    })
  end)

  local samples, res, name_prefix
  if quality then 
    samples = 4 
    res = 800
    name_prefix = "qmat_"
  else
    samples = 1
    res = 600
    name_prefix = "mat_"
  end

  opts = {
    sampler = stratified_sampler {
      samples_per_x = samples,
      samples_per_y = samples,
    },
    filter = mitchell_filter {
      radius = 1.5,
    },
    x_res = res / 2,
    y_res = res,
  }

  if render_opts then
    for k,v in pairs(render_opts) do
      opts[k] = v
    end
  end

  write_png_image(name_prefix .. name .. ".png", render(scene, opts))
end

for _, sigma in pairs({0, 1, 0.1}) do
  test_scene("matte_" .. sigma * 100, function()
    return matte_material { color = rgb(1,1,1) * 0.8, sigma = sigma }
  end)
end

for _, roughness in pairs({0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.6, 0.8}) do
  test_scene("plastic_" .. roughness * 100, function()
    return plastic_material {
      color = rgb(0.9, 0.7, 0.5),
      reflect_color = rgb(1, 1, 1) * 0.9,
      eta = 1.5,
      roughness = roughness,
    }
  end)
end

test_scene("mirror", function()
  return mirror_material { color = rgb(0.9, 0.9, 0.9) }
end)

for _, eta in pairs({0.8, 1.5, 2.0, 3.0}) do
  test_scene("glass_" .. eta * 100, function()
    return glass_material {
      color = rgb(1,1,1) * 0.9,
      transmit_color = rgb(1,1,1) * 0.9,
      eta = eta
    }
  end)
end

for _, roughness in pairs({0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.6, 0.8}) do
  test_scene("metal_" .. roughness*100, function()
    return metal_material {
      color = rgb(0.9, 0.8, 0.5),
      eta = 1.07,
      k = 6.7,
      roughness = roughness,
    }
  end)
end

for _, roughness in pairs({0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.6, 0.8}) do
  opts = {}
  opts["renderer"] = "path"
  if quality then
    opts["sampler"] = stratified_sampler {
      samples_per_x = 6,
      samples_per_y = 6,
    }
  else
    opts["sampler"] = stratified_sampler {
      samples_per_x = 2,
      samples_per_y = 2,
    }
  end

  test_scene("rough_glass_" .. roughness*100, function()
    return rough_glass_material {
      color = rgb(1,1,1) * 0.9,
      transmit_color = rgb(1,1,1) * 0.9,
      eta = 1.5,
      roughness = roughness,
    }
  end, opts)
end
