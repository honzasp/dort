local b = dort.builder
local g = dort.geometry

function dort.builder.block(B, callback)
  b.push_state(B)
  local result = callback(B)
  b.pop_state(B)
  return result
end

function dort.builder.frame(B, callback)
  b.push_frame(B)
  callback(B)
  return b.pop_frame(B)
end

function dort.geometry.rotate_x_around(angle, origin)
  return 
    g.translate(origin:x(), origin:y(), origin:z()) *
    g.rotate_x(angle) *
    g.translate(-origin:x(), -origin:y(), -origin:z())
end

function dort.geometry.rotate_y_around(angle, origin)
  return 
    g.translate(origin:x(), origin:y(), origin:z()) *
    g.rotate_y(angle) *
    g.translate(-origin:x(), -origin:y(), -origin:z())
end

function dort.geometry.rotate_z_around(angle, origin)
  return 
    g.translate(origin:x(), origin:y(), origin:z()) *
    g.rotate_z(angle) *
    g.translate(-origin:x(), -origin:y(), -origin:z())
end
