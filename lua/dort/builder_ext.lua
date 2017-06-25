--- @module dort.builder
local b = dort.builder

--- Isolate changes in builder state.
-- Execute `callback` between `push_state` and `pop_state`, and return the
-- return value of the callback.
-- @function block
-- @param B
-- @param callback
-- @within State
function dort.builder.block(B, callback)
  b.push_state(B)
  local result = callback(B)
  b.pop_state(B)
  return result
end

--- Create an aggregate primitive.
-- Execute `callback` between `push_frame` and `pop_frame` and return the
-- produced primitive.
-- @function frame
-- @param B
-- @param callback
-- @within Frame
function dort.builder.frame(B, callback)
  b.push_frame(B)
  callback(B)
  return b.pop_frame(B)
end

--- Add a diffuse light.
-- Creates a diffuse light with passed `params` and add the shape with the light
-- to the scene.
-- @function add_diffuse_light
-- @param B
-- @param params
-- @within Adding
function dort.builder.add_diffuse_light(B, params)
  local shape = params.shape
  local transform = params.transform
  local light = dort.light.make_diffuse(params)
  b.block(B, function()
    b.set_transform(B, transform)
    b.set_material(B, dort.material.make_lambert { albedo = dort.spectrum.rgb(0) })
    b.add_shape(B, shape, light)
  end)
  b.add_light(B, light)
end

