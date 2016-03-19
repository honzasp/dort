require "minecraft/anvil"
require "minecraft/read"
require "minecraft/blocks"

function minecraft.add_world(params)
  local block_grid = dort.grid.make()
  local voxel_grid = dort.grid.make()
  local anvil_map = minecraft.anvil.open_map(params.map)
  minecraft.read.map_to_blocks(block_grid, anvil_map, params.box)
  minecraft.anvil.close_map(anvil_map)

  minecraft.blocks.blocks_to_voxels(voxel_grid, block_grid, params.box)
  dort.builder.add_voxel_grid {
    grid = voxel_grid,
    box = params.box,
    voxel_materials = minecraft.blocks.voxel_materials(),
  }
end
