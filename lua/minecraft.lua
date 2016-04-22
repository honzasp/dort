require "minecraft/anvil"
require "minecraft/read"

function minecraft.add_world(builder, params)
  local block_grid = dort.grid.make()
  local voxel_grid = dort.grid.make()
  local anvil_map = minecraft.anvil.open_map(params.map)
  minecraft.read.map_to_blocks(block_grid, anvil_map, params.box)
  minecraft.anvil.close_map(anvil_map)

  local world = build_world(builder, block_grid, params.box, params)
  dort.builder.add_voxel_grid(builder, {
    box = params.box,
    grid = world.voxel_grid,
    cube_voxels = world.cube_voxels,
    primitive_voxels = world.primitive_voxels,
  })
end

local World = {}
World.__index = World

local DEFAULT_OPTIONS = {
  torch_num_samples = 1,
}

function build_world(builder, block_grid, box, params)
  local world = {};
  setmetatable(world, World)

  world.voxel_grid = dort.grid.make()
  world.block_grid = block_grid
  world.builder = builder
  world.block_funs = {}
  world.block_names = {}

  world.options = {}
  for key, value in pairs(DEFAULT_OPTIONS) do
    if params[key] then
      world.options[key] = params[key]
    else
      world.options[key] = value
    end
  end

  local white = dort.material.make_matte {
    color = dort.spectrum.rgb(1, 1, 1)
  }
  world.unknown_voxel = 1
  world.cube_voxels = {
    {white, white, white, white, white, white},
  }
  world.primitive_voxels = {}

  ;(require "minecraft/blocks/dirts")(world)
  ;(require "minecraft/blocks/stones")(world)
  ;(require "minecraft/blocks/plants")(world)
  ;(require "minecraft/blocks/utility")(world)
  ;(require "minecraft/blocks/water")(world)

  for z = box:min():z(), box:max():z() - 1 do
    for y = box:min():y(), box:max():y() - 1 do
      for x = box:min():x(), box:max():x() - 1 do
        local pos = dort.geometry.vec3i(x, y, z)
        world.voxel_grid:set(pos, world:block_to_voxel(world:block(pos)))
      end
    end
  end

  return world
end

function World:block(pos)
  local block_int = self.block_grid:get(pos)
  local block_id = minecraft.read.block_id(block_int)
  local block_data = minecraft.read.block_data(block_int)
  return { pos = pos, id = block_id, data = block_data }
end

function World:option(key)
  return self.options[key]
end

function World:block_to_voxel(block)
  if block.id == 0 then
    return 0
  end

  local block_fun = self.block_funs[block.id]
  if block_fun then
    return block_fun(block)
  else
    return self.unknown_voxel
  end
end

function World:define_cube_voxel(material)
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

  self.cube_voxels[#self.cube_voxels + 1] = voxel_material
  return #self.cube_voxels
end

function World:define_primitive_voxel(primitive)
  self.primitive_voxels[#self.primitive_voxels + 1] = primitive
  return -#self.primitive_voxels
end

function World:define_block(block_name, block_id, voxel_or_fun)
  if self.block_funs[block_id] then
    error("Duplicate definition of block " .. block_id 
      .. " (" .. self.block_names[block_id] .. " and " .. block_name .. ")")
  end

  local fun = voxel_or_fun
  if type(voxel_or_fun) ~= "function" then
    fun = function() return voxel_or_fun end
  end

  self.block_funs[block_id] = fun
  self.block_names[block_id] = block_name
end
