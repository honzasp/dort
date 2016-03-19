require "minecraft/anvil"
require "minecraft/read"
require "minecraft/decode"

local box = dort.geometry.boxi(
  dort.geometry.vec3i(-200, 0, -200),
  dort.geometry.vec3i(200, 256, 200))

local home = os.getenv("HOME")
local map = minecraft.anvil.open_map(home .. "/.minecraft/saves/Dev world")
local block_grid = minecraft.read.box(map, box)
minecraft.anvil.close_map(map)

local voxel_grid = minecraft.decode.box(block_grid, box)

local scene = dort.builder.define_scene(function()
  dort.builder.add_voxel_grid {
    grid = voxel_grid,
    box = box,
  }

  dort.builder.add_light(dort.light.make_infinite {
    radiance = dort.spectrum.rgb(1, 1, 1) * 0.5,
    num_samples = 4,
  })

  dort.builder.set_camera(dort.camera.make_perspective {
    transform = dort.geometry.look_at(
        dort.geometry.point(-200, 200, -200),
        dort.geometry.point(0, 64, 0),
        dort.geometry.vector(0, 1, 0)) *
      dort.geometry.scale(-1, 1, 1) ,
    fov = dort.math.pi / 3,
  })
end)
minecraft.anvil.close_map(map)

dort.image.write_png("minecraft_chunk.png", dort.builder.render(scene, {
  x_res = 800,
  y_res = 800,
  sampler = dort.sampler.make_stratified {
    samples_per_x = 2,
    samples_per_y = 2,
  },
  filter = dort.filter.make_lanczos_sinc {
    radius = 1.2,
  },
}))
