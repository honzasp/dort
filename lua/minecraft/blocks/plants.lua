local _ENV = require "minecraft/blocks/_env"
return function(world)
  local B = world.builder

  local wood_voxels = {
    world:define_cube_voxel(m.make_matte {
      color = rgbh("665132"),
    }),
  }

  world:define_block("log", 17, function(block)
    return wood_voxels[(block.data & 3) + 1]
  end)
  world:define_block("log2", 162, function(block)
    return wood_voxels[(block.data & 3) + 5]
  end)

  do
    local oak_leaf_colors = {
      rgbh("2b6006"),
      rgbh("3d8011"),
      rgbh("488c1b"),
      rgbh("58962e"),
      rgbh("486006"),
      rgbh("57720e"),
      rgbh("617b19"),
      rgbh("506006"),
      rgbh("5c6d0e"),
      rgbh("667718"),
    }

    local oak_leaf_materials = {}
    for i = 1, #oak_leaf_colors do
      oak_leaf_materials[i] = m.make_matte { color = oak_leaf_colors[i] }
    end

    function generate_oak_leaf(B, rng)
      function make_lobe(lobe, lobes, x_sign)
        local y = (lobe / (lobes + 1)) + 3/100 * rng()
        local x
        if lobe % 2 == 0 then
          x = 3/20 + 3/10 * ((lobe - 1) / (lobes - 1)) * (0.85 + 0.3 * rng())
        else
          x = 1/20 + 1/5 * ((lobe - 1) / (lobes - 1)) * (0.85 + 0.3 * rng())
        end
        return g.vec2(x * x_sign, y)
      end

      local lobes = floor(rng() * 4 + 5)
      if lobes % 2 == 0 then
        lobes = lobes + 1
      end

      local vertices = {}

      vertices[1] = g.vec2(0, 0)
      for lobe = 1, lobes do
        vertices[#vertices + 1] = make_lobe(lobe, lobes, 1)
      end
      vertices[#vertices + 1] = g.vec2(0, 1)
      for lobe = 0, lobes - 1 do
        vertices[#vertices + 1] = make_lobe(lobes - lobe, lobes, -1)
      end

      b.set_material(B, oak_leaf_materials[1 + floor(#oak_leaf_materials * rng())])
      b.add_shape(B, s.make_polygon {
        vertices = vertices,
      })
    end

    function sample_sphere(u1, u2)
      local z = 1 - 2 * u1;
      local r = sqrt(max(0, 1 - z * z));
      local phi = 2 * pi * u2;
      local x = cos(phi) * r;
      local y = sin(phi) * r;
      return g.vector(x, y, z);
    end

    function generate_leaves(B, seed, generate_leaf)
      return world:define_primitive_voxel(b.frame(B, function()
        local rng = make_rng(seed)
        for i = 1, 200 do
          b.block(B, function()
            local origin = g.point(rng(), rng(), rng())
            local dir1 = sample_sphere(rng(), rng())
            local dir2 = sample_sphere(rng(), rng())
            b.set_transform(B, g.look_at(origin, origin + dir1, dir2))
            b.set_transform(B, g.scale(0.2))
            generate_leaf(B, rng)
          end)
        end
      end))
    end

    local leaves = {
      generate_oak_leaf,
    }

    local leaves_voxels = {}
    for leaf_type = 1, #leaves do
      leaves_voxels[leaf_type] = {}
      for i = 1, 10 do
        leaves_voxels[leaf_type][i] = generate_leaves(B, i * 1234, leaves[leaf_type])
      end
    end

    world:define_block("leaves", 18, function(block)
      local leaf_type = (block.data & 3) + 1
      local voxels = leaves_voxels[leaf_type]
      local i = (2 * block.pos:x() + 3 * block.pos:y() + 5 * block.pos:z()) % #voxels
      return voxels[i + 1]
    end)
  end
end
