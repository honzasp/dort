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

function dort.builder.add_diffuse_light(B, params)
  local shape = params.shape
  local light = dort.light.make_diffuse(params)
  b.add_shape(B, shape, light)
  b.add_light(B, light)
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

function dort.geometry.scale_around(scale, origin)
  return g.translate(origin:x(), origin:y(), origin:z()) *
    g.scale(scale) *
    g.translate(-origin:x(), -origin:y(), -origin:z())
end

function dort.material.make_phong(params)
  -- TODO: implement real Phong!
  return dort.material.make_matte { 
    color = params.color,
  }
end
