require "minecraft/anvil"

local home = os.getenv("HOME")
local map = minecraft.anvil.open_map(home .. "/.minecraft/saves/Dev world")

local chunk_xzs = {}
for x = 0, 4 do
  for z = 0, 4 do
    chunk_xzs[#chunk_xzs + 1] = {x, z}
  end
end

local scene = dort.builder.define_scene(function()
  for _, chunk_xz in ipairs(chunk_xzs) do
    local chunk = minecraft.anvil.read_chunk(map, chunk_xz[1], chunk_xz[2])
    if not chunk then
      print("chunk " .. chunk_xz[1] .. ", " .. chunk_xz[2] .. " is not generated")
      goto skip_chunk
    end

    local chunk_x = chunk["Level"]["xPos"]
    local chunk_z = chunk["Level"]["zPos"]

    for _, section in ipairs(chunk["Level"]["Sections"]) do
      local section_y = section["Y"]
      local voxels = {}

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

        local voxel = block_id
        voxels[1 + block_x + block_y * 16 + block_z * 16 * 16] = voxel
      end

      dort.builder.define_block(function()
        dort.builder.set_transform(
          dort.geometry.translate(16 * chunk_x, 16 * section_y, 16 * chunk_z))
        dort.builder.add_voxel_grid({
          extent_x = 16,
          extent_y = 16,
          extent_z = 16,
          voxels = voxels,
        })
      end)
    end

    ::skip_chunk::
  end

  dort.builder.add_light(dort.light.make_infinite {
    radiance = dort.spectrum.rgb(1, 1, 1) * 0.5,
    num_samples = 1,
  })

  dort.builder.set_camera(dort.camera.make_perspective {
    transform = dort.geometry.look_at(
        dort.geometry.point(-40, 100, 16 * 4 + 40),
        dort.geometry.point(16 * 2.5, 64, 16 * 2.5),
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
