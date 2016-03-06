for angle = 0, 360, 30 do
  local scene = define_scene(function()
    material(matte_material {
      color = rgb(1, 1, 1)
    })

    block(function()
      transform(rotate_y(angle / 180 * pi))

      add_voxel_grid {
        extent_x = 3,
        extent_y = 3,
        extent_z = 2,
        voxels = {
          1,1,1, 1,0,1, 1,0,0,
          1,0,0, 1,0,0, 0,0,1,
        }
      }
    end)

    add_light(infinite_light {
      radiance = rgb(1,1,1) * 0.5,
      num_samples = 4,
    })

    camera(perspective_camera {
      transform = look_at(
        point(1.5, 5, 10),
        point(1.5, 1.5, 1),
        vector(0, 1, 0)),
      fov = pi / 4,
    })
  end)

  write_png_image("voxels_" .. angle .. ".png", render(scene, {
    x_res = 400,
    y_res = 400,
  }))
end
