require "minecraft/anvil"

local home = os.getenv("HOME")
local map = minecraft.anvil.open_map(home .. "/.minecraft/saves/Dev world")
local nbt = minecraft.anvil.read_chunk(map, 2, 3)
minecraft.anvil.close_map(map)

local chunk_x = nbt["Level"]["xPos"]
local chunk_z = nbt["Level"]["zPos"]
print("chunk " .. chunk_x .. " " .. chunk_z)

for _, section in ipairs(nbt["Level"]["Sections"]) do
  local section_y = section["Y"]
  print("section " .. section_y)
  for i = 0, 4096 - 1 do
    local block_x = i & 0xf
    local block_z = (i >> 4) & 0xf
    local block_y = (i >> 8) & 0xf
    local x = (chunk_x << 4) + block_x
    local z = (chunk_z << 4) + block_z
    local y = (section_y << 4) + block_y

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

    local block_id = block_byte + (block_nibble << 8)
    local block_data = data_nibble

    print("  block " ..  x .. " " .. z .. " " .. y
      .. ": " .. block_id .. " (" .. block_data .. ")")
  end
end
