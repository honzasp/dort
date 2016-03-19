local anvil = {}
if not minecraft then minecraft = {} end
minecraft.anvil = anvil

local function open_region(map, region_x, region_z)
  local region = {}
  region.path = map.path .. "/region/r." ..
    region_x .. "." .. region_z .. ".mca"
  region.chunk_offsets = {}

  local file, err = io.open(region.path, "r")
  if not file then
    error("Could not read Anvil region: " .. err)
  end
  region.file = file

  local location_table = region.file:read(1024 * 4)
  for i = 0, 1024 - 1 do
    local b0 = location_table:byte(4 * i + 1)
    local b1 = location_table:byte(4 * i + 2)
    local b2 = location_table:byte(4 * i + 3)
    local b3 = location_table:byte(4 * i + 4)
    region.chunk_offsets[i + 1] = (b0 << 16) + (b1 << 8) + b2
  end

  return region
end

local function close_region(region)
  region.file:close()
end

local function read_chunk_from_region(region, chunk_x, chunk_z)
  local chunk_idx = chunk_x + 32 * chunk_z
  local sector_begin = region.chunk_offsets[chunk_idx + 1]
  if sector_begin == 0 then
    return nil
  end

  region.file:seek("set", sector_begin * 4096)
  local header = region.file:read(5)
  local length, ztype = string.unpack(">I4I1", header)
  if length > 8*1024*1024 then
    error("Chunk is suspiciously long: " .. length .. " bytes")
  end

  local unzipped_chunks = {}
  local remaining_bytes = length - 1
  local inflater = zlib.inflate()
  while remaining_bytes > 0 do
    local chunk_size = 4*1024
    local zipped_chunk = region.file:read(chunk_size)
    local unzipped_chunk, eof = inflater(zipped_chunk)
    unzipped_chunks[#unzipped_chunks + 1] = unzipped_chunk
    remaining_bytes = remaining_bytes - chunk_size
    if eof then break end
  end

  local unzipped_bytes = table.concat(unzipped_chunks)
  return dort.nbt.read(unzipped_bytes)
end

function anvil.open_map(path)
  local map = {}
  map.path = path
  map.regions = {}
  return map
end

function anvil.close_map(map)
  for _, region in pairs(map.regions) do
    close_region(region)
  end
  map.regions = {}
end

function anvil.read_chunk(map, chunk_x, chunk_z)
  local region_x = chunk_x // 32
  local region_z = chunk_z // 32
  local region_id = {region_x, region_z}
  local region = map.regions[region_id]

  if not region then
    region = open_region(map, region_x, region_z)
    map.regions[region_id] = region
  end

  return read_chunk_from_region(region, chunk_x & 0x1f, chunk_z & 0x1f)
end

return anvil
