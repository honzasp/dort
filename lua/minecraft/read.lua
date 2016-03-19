local read = {}
minecraft = minecraft or {}
minecraft.read = read

function read.pack_block(block_id, block_data)
  return block_id + (block_data << 12)
end

function read.block_id(block)
  return block & 0xfff
end

function read.block_data(block)
  return (block >> 12) & 0xf
end

local function read_section(block_grid, chunk_x, chunk_z, section)
  local section_y = section["Y"]

  for i = 0, 4096 - 1 do
    local block_x = i & 0xf
    local block_z = (i >> 4) & 0xf
    local block_y = (i >> 8) & 0xf
    local x = (chunk_x << 4) + block_x
    local z = (chunk_z << 4) + block_z
    local y = (section_y << 4) + block_y
    local pos = dort.geometry.vec3i(x, y, z)

    local block_byte = string.byte(section["Blocks"], i + 1);
    local block_nibble = 0;
    if section["Add"] then
      local block_extra_byte = string.byte(section["Add"], (i >> 1) + 1)
      if i & 1 == 0 then
        block_nibble = block_extra_byte & 0xf
      else
        block_nibble = (block_extra_byte >> 4) & 0xf
      end
    end

    local data_byte = string.byte(section["Data"], (i >> 1) + 1)
    local data_nibble
    if i & 1 == 0 then
      data_nibble = data_byte & 0xf
    else
      data_nibble = (data_byte >> 4) & 0xf
    end

    local block = read.pack_block(block_byte + (block_nibble << 8), data_nibble)
    block_grid:set(pos, block)
  end
end

local function read_chunk(block_grid, anvil_map, chunk_x, chunk_z)
  local chunk = minecraft.anvil.read_chunk(anvil_map, chunk_x, chunk_z)
  if not chunk then
    print("chunk " .. chunk_x .. ", " .. chunk_z .. " was not found")
    return
  end

  assert(chunk_x == chunk["Level"]["xPos"])
  assert(chunk_z == chunk["Level"]["zPos"])

  for _, section in ipairs(chunk["Level"]["Sections"]) do
    read_section(block_grid, chunk_x, chunk_z, section)
  end
end

function read.box(anvil_map, box)
  local block_grid = dort.grid.make()
  return read.box_to_grid(block_grid, anvil_map, box)
end

function read.box_to_grid(block_grid, anvil_map, box)
  local min_chunk_x = math.floor(box:min():x() / 16)
  local min_chunk_z = math.floor(box:min():z() / 16)
  local max_chunk_x = math.ceil(box:max():x() / 16)
  local max_chunk_z = math.ceil(box:max():z() / 16)

  for chunk_z = min_chunk_z, max_chunk_z do
    for chunk_x = min_chunk_x, max_chunk_x do
      read_chunk(block_grid, anvil_map, chunk_x, chunk_z)
    end
  end
  return block_grid
end

return read
