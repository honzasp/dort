local _ENV = require "minecraft/blocks/_env"
return function(world)
  local B = world.builder

  local wood_voxels = {
    world:define_cube_voxel(m.make_matte {
      color = rgbh("665132"),
    }),
    world:define_cube_voxel(m.make_matte {
      color = rgbh("2e1d0c"),
    }),
    world:define_cube_voxel(m.make_matte {
      color = rgbh("cfcec9"),
    })
  }

  world:define_block("log", 17, function(block)
    return wood_voxels[(block.data & 3) + 1]
  end)
  world:define_block("log2", 162, function(block)
    return wood_voxels[(block.data & 3) + 5]
  end)

  function leaf_materials(colors) 
    local materials = {}
    for i = 1, #colors do
      materials[i] = m.make_matte { color = colors[i] }
    end
    return materials
  end

  function generate_lobed_leaf(B, rng, lobes, make_lobe, materials)
    local vertices = {}
    vertices[1] = g.vec2(0, 0)
    for lobe = 1, lobes do
      vertices[#vertices + 1] = make_lobe(lobe, 1)
    end
    vertices[#vertices + 1] = g.vec2(0, 1)
    for lobe = 0, lobes - 1 do
      vertices[#vertices + 1] = make_lobe(lobes - lobe, -1)
    end

    b.set_material(B, materials[1 + floor(#materials * rng())])
    b.add_shape(B, s.make_polygon {
      vertices = vertices,
    })
  end

  local oak_leaf_materials = leaf_materials {
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

  function generate_oak_leaf(B, rng)
    local lobes = floor(rng() * 4 + 5)
    if lobes % 2 == 0 then
      lobes = lobes + 1
    end
    function make_lobe(lobe, x_sign)
      local y = (lobe / (lobes + 1)) + 3/100 * rng()
      local x
      if lobe % 2 == 0 then
        x = 3/20 + 3/10 * ((lobe - 1) / (lobes - 1)) * (0.85 + 0.3 * rng())
      else
        x = 1/20 + 1/5 * ((lobe - 1) / (lobes - 1)) * (0.85 + 0.3 * rng())
      end
      return g.vec2(x * x_sign, y)
    end
    generate_lobed_leaf(B, rng, lobes, make_lobe, oak_leaf_materials)
  end

  local spruce_leaf_materials = leaf_materials {
    rgbh("3a852f"),
    rgbh("3b7633"),
    rgbh("468e3c"),
    rgbh("578e3c"),
    rgbh("5b9b3b"),
    rgbh("4f8e2f"),
    rgbh("359b35"),
    rgbh("2d8f2d"),
    rgbh("2d8f5f"),
    rgbh("379b6b"),
    rgbh("42986f"),
  }

  local spruce_leaf_shape = s.make_polygon {
    vertices = {
      g.vec2(0, 1),
      g.vec2(-1/16, 0),
      g.vec2(1/16, 0),
    }
  }

  function generate_spruce_leaf(B, rng)
    b.set_material(B, spruce_leaf_materials[1 + floor(#spruce_leaf_materials * rng())])
    b.add_shape(B, spruce_leaf_shape)
  end

  local birch_leaf_materials = leaf_materials {
    rgbh("63a423"),
    rgbh("74b831"),
    rgbh("75b03b"),
    rgbh("6ca03a"),
    rgbh("629134"),
    rgbh("59a423"),
    rgbh("549822"),
    rgbh("518e25"),
    rgbh("5d9633"),
    rgbh("6ba740"),
    rgbh("74b147"),
    rgbh("85b147"),
    rgbh("81b03f"),
    rgbh("7dae37"),
    rgbh("7eb52f"),
  }

  function generate_birch_leaf(B, rng)
    local lobes = floor(rng() * 2.5 + 1)
    function make_lobe(lobe, x_sign)
      local y = lobe / (lobes + 2) + 1/20 * rng()
      local x = 1/2 * sin(pi * (lobes - lobe + 1) / (lobes + 4)) + 1/10 * rng()
      return g.vec2(x * x_sign, y)
    end
    generate_lobed_leaf(B, rng, lobes, make_lobe, birch_leaf_materials)
  end

  function sample_sphere(u1, u2)
    local z = 1 - 2 * u1;
    local r = sqrt(max(0, 1 - z * z));
    local phi = 2 * pi * u2;
    local x = cos(phi) * r;
    local y = sin(phi) * r;
    return g.vector(x, y, z);
  end

  function generate_leaves_voxels(B, density, scale, generate_leaf)
    local voxels = {}
    for i = 1, 7 do
      voxels[i] = world:define_primitive_voxel(b.frame(B, function()
        local rng = make_rng(i * 1234 + 56789)
        for leaf = 1, density do
          b.block(B, function()
            local origin = g.point(rng(), rng(), rng())
            local dir1 = sample_sphere(rng(), rng())
            local dir2 = sample_sphere(rng(), rng())
            b.set_transform(B, g.look_at(origin, origin + dir1, dir2))
            b.set_transform(B, g.scale(scale))
            generate_leaf(B, rng)
          end)
        end
      end))
    end
    return voxels
  end

  local leaves_voxels = {
    generate_leaves_voxels(B, 200, 0.2, generate_oak_leaf),
    generate_leaves_voxels(B, 5000, 0.1, generate_spruce_leaf),
    generate_leaves_voxels(B, 150, 0.15, generate_birch_leaf),
  }

  world:define_block("leaves", 18, function(block)
    local leaf_type = (block.data & 3) + 1
    local voxels = leaves_voxels[leaf_type]
    local i = (2 * block.pos:x() + 3 * block.pos:y() + 5 * block.pos:z()) % #voxels
    return voxels[i + 1]
  end)
end
