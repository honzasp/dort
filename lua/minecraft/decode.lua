require "minecraft/read"

local decode = {}
minecraft = minecraft or {}
minecraft.decode = decode

function decode.box(block_grid, box)
  local voxel_grid = dort.grid.make()
  return decode.box_to_grid(voxel_grid, block_grid, box)
end

function decode.box_to_grid(voxel_grid, block_grid, box)
  for z = box:min():z(), box:max():z() - 1 do
    for y = box:min():y(), box:max():y() - 1 do
      for x = box:min():x(), box:max():x() - 1 do
        local pos = dort.geometry.vec3i(x, y, z)
        local block_id = minecraft.read.block_id(block_grid:get(pos))

        local voxel
        if block_id == 0 then
          voxel = 0
        else
          voxel = 1
        end
        voxel_grid:set(pos, voxel)
      end
    end
  end
  return voxel_grid
end

return decode
