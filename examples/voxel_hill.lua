--for angle = 0, 360, 30 do
do local angle = 30
  local scene = define_scene(function()
    material(matte_material {
      color = rgb(1, 1, 1)
    })

    block(function()
      local s = 1
      local ex = 32*s
      local ey = 16*s
      local ez = 32*s
      transform(rotate_z(angle / 180 * pi) * scale(300 / (ex + ey + ez))
        * translate(-ex/2, -ey/2, -ez/2))

      local voxels = {}
      for z = 0, ez - 1 do
        for y = 0, ey - 1 do
          for x = 0, ex - 1 do
            local fx = 0.5 + 0.5*sin(x / ex * 4*pi)
            local fz = 0.5 + 0.5*sin(z / ez * 5*pi)
            local f = 0.5 * (fx + fz)
            local h = f * ey * 0.5
            if y > h then
              voxels[#voxels + 1] = 0
            else
              voxels[#voxels + 1] = 1
            end
          end
        end
      end

      add_voxel_grid {
        extent_x = ex,
        extent_y = ey,
        extent_z = ez,
        voxels = voxels,
      }
      print(ex * ey * ez .. " voxels")
    end)

    add_light(infinite_light {
      radiance = rgb(1,1,1) * 0.5,
      num_samples = 8,
    })

    camera(perspective_camera {
      transform = look_at(
        point(0, 200, 20),
        point(0, 0, 0),
        vector(0, 1, 0)),
      fov = pi / 4,
    })
  end)

  write_png_image("hill_" .. angle .. ".png", render(scene, {
    x_res = 400,
    y_res = 400,
    sampler = stratified_sampler {
      samples_per_x = 2,
      samples_per_y = 2,
    },
    filter = mitchell_filter {
      radius = 1.5
    },
  }))
end
