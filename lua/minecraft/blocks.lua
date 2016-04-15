require "minecraft/read"

local blocks = {}
minecraft.blocks = blocks

local BLOCKS = {}
local BLOCK_NAMES = {}

local white = dort.material.make_matte {
  color = dort.spectrum.rgb(1, 1, 1),
}
local VOXEL_MATERIALS = {
  {white, white, white, white, white, white},
}
local VOXEL_PRIMITIVES = {}
local UNKNOWN_VOXEL = 1

function blocks.voxel(material)
  local voxel_material
  if type(material) == "table" then
    local east = material.east or material.east_west or material.side
    local west = material.west or material.east_west or material.side
    local south = material.south or material.south_north or material.side
    local north = material.north or material.south_north or material.side
    local top = material.top or material.top_bottom or material.side
    local bottom = material.bottom or material.top_bottom or material.side

    local function check(face, mat)
      if not mat then 
        error("Material for " .. face .. " is not defined")
      end
    end
    check("east", east)
    check("west", west)
    check("south", south)
    check("north", north)
    check("top", top)
    check("bottom", bottom)

    voxel_material = {
      east, top, south,
      west, bottom, north,
    }
  else
    voxel_material = {
      material, material, material,
      material, material, material,
    }
  end

  VOXEL_MATERIALS[#VOXEL_MATERIALS + 1] = voxel_material
  return #VOXEL_MATERIALS
end

function blocks.define(block_name, block_id, material_or_fun)
  if BLOCKS[block_id] then
    error("Duplicate definition of block " .. block_id 
      .. " (" .. BLOCK_NAMES[block_id] .. ")")
  end

  local fun = material_or_fun
  if type(material_or_fun) ~= "function" then
    local voxel = blocks.voxel(material_or_fun)
    fun = function() return voxel end
  end
  BLOCKS[block_id] = fun
  BLOCK_NAMES[block_id] = block_name
end

function blocks.voxel_primitive(prim)
  VOXEL_PRIMITIVES[#VOXEL_PRIMITIVES + 1] = prim
  return -#VOXEL_PRIMITIVES
end

function blocks.define_primitive(block_name, block_id, prim)
  local voxel = blocks.voxel_primitive(prim)
  blocks.define(block_name, block_id, function() return voxel end)
end

function blocks.voxel_materials()
  return VOXEL_MATERIALS
end

function blocks.voxel_primitives()
  return VOXEL_PRIMITIVES
end

function block_to_voxel(block_pos, block_id, block_data)
  if block_id == 0 then
    return 0
  end

  local block = BLOCKS[block_id]
  if block then
    return block { pos = block_pos, id = block_id, data = block_data }
  else
    return UNKNOWN_VOXEL
  end
end

function blocks.blocks_to_voxels(voxel_grid, block_grid, box)
  for z = box:min():z(), box:max():z() - 1 do
    for y = box:min():y(), box:max():y() - 1 do
      for x = box:min():x(), box:max():x() - 1 do
        local pos = dort.geometry.vec3i(x, y, z)
        local block = block_grid:get(pos)
        local block_id = minecraft.read.block_id(block)
        local block_data = minecraft.read.block_data(block)
        voxel_grid:set(pos, block_to_voxel(pos, block_id, block_data))
      end
    end
  end
  return voxel_grid
end

require "minecraft/blocks/dirts"
require "minecraft/blocks/stones"
require "minecraft/blocks/utility"

return decode
