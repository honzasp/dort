--- @module dort.geometry
local g = dort.geometry

--- Rotation around `origin` by `angle` through the X axis.
-- @function rotate_x_around
-- @param angle
-- @param origin
-- @within Transform
function dort.geometry.rotate_x_around(angle, origin)
  return 
    g.translate(origin:x(), origin:y(), origin:z()) *
    g.rotate_x(angle) *
    g.translate(-origin:x(), -origin:y(), -origin:z())
end

--- Rotation around `origin` by `angle` through the Y axis.
-- @function rotate_y_around
-- @param angle
-- @param origin
-- @within Transform
function dort.geometry.rotate_y_around(angle, origin)
  return 
    g.translate(origin:x(), origin:y(), origin:z()) *
    g.rotate_y(angle) *
    g.translate(-origin:x(), -origin:y(), -origin:z())
end

--- Rotation around `origin` by `angle` through the Z axis.
-- @function rotate_z_around
-- @param angle
-- @param origin
-- @within Transform
function dort.geometry.rotate_z_around(angle, origin)
  return 
    g.translate(origin:x(), origin:y(), origin:z()) *
    g.rotate_z(angle) *
    g.translate(-origin:x(), -origin:y(), -origin:z())
end

--- Scale based in `origin`.
-- @function scale_around
-- @param scale
-- @param origin
-- @within Transform
function dort.geometry.scale_around(scale, origin)
  return g.translate(origin:x(), origin:y(), origin:z()) *
    g.scale(scale) *
    g.translate(-origin:x(), -origin:y(), -origin:z())
end
