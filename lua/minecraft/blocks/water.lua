local _ENV = require "minecraft/blocks/_env"
return function(world)
  local B = world.builder

  function water_mesh(height) 
    return s.make_mesh {
      points = {
        g.point(0, 0, 0),
        g.point(1, 0, 0),
        g.point(1, 0, 1),
        g.point(0, 0, 1),
        g.point(0, height, 0),
        g.point(1, height, 0),
        g.point(1, height, 1),
        g.point(0, height, 1),
      },
      vertices = {
        4, 7, 6,  6, 5, 4, -- up
        0, 1, 2,  2, 3, 0, -- down
        0, 1, 5,  5, 4, 0, -- north
        3, 2, 6,  6, 7, 3, -- south
        0, 3, 7,  7, 4, 0, -- west
        1, 2, 6,  6, 5, 1, -- east
      }
    }
  end

  local full_water_mesh = water_mesh(1)
  local surface_water_mesh = water_mesh(15/16)
  local water_material = m.make_matte { 
    color = rgb(0.5, 0.5, 1),
  }

  function generate_water_voxel(B, sides)
    if sides == 0 then
      return 0
    end

    local up = (sides & 1) ~= 0
    local down = (sides & 2) ~= 0
    local north = (sides & 4) ~= 0
    local south = (sides & 8) ~= 0
    local west = (sides & 16) ~= 0
    local east = (sides & 32) ~= 0

    local mesh
    if up then 
      mesh = surface_water_mesh 
    else 
      mesh = full_water_mesh 
    end

    return world:define_primitive_voxel(b.frame(B, function()
      b.set_material(B, water_material)

      function side(idx)
        b.add_triangle(B, mesh, 6 * idx)
        b.add_triangle(B, mesh, 6 * idx + 3)
      end

      if up then side(0) end
      if down then side(1) end
      if north then side(2) end
      if south then side(3) end
      if west then side(4) end
      if east then side(5) end
    end))
  end

  local water_voxels = {}
  for sides = 0, 63 do
    water_voxels[sides + 1] = generate_water_voxel(B, sides)
  end

  function water_block(block)
    if block.data ~= 0 then
      return 0
    end

    function isnt_side_water(x, y, z)
      return not is_water(world:block(block.pos + g.vec3i(x, y, z)))
    end

    local sides = 0
    if isnt_side_water(0, 1, 0) then sides = sides + 1 end
    if isnt_side_water(0, -1, 0) then sides = sides + 2 end
    if isnt_side_water(0, 0, -1) then sides = sides + 4 end
    if isnt_side_water(0, 0, 1) then sides = sides + 8 end
    if isnt_side_water(-1, 0, 0) then sides = sides + 16 end
    if isnt_side_water(1, 0, 0) then sides = sides + 32 end

    return water_voxels[sides + 1]
  end

  function is_water(block)
    return (block.id == 9 or block.id == 8) and block.data == 0
  end

  world:define_block("water", 9, water_block)
  world:define_block("flowing_water", 8, water_block)
end
