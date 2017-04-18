local dort = require "dort"
local json = require "dkjson"

local function read_vector(json)
  return dort.geometry.vector(json[1], json[2], json[3])
end

local function read_point(json)
  return dort.geometry.point(json[1], json[2], json[3])
end

local function read_spectrum(json)
  if type(json) == "number" then
    return dort.spectrum.rgb(json)
  else
    return dort.spectrum.rgb(json[1], json[2], json[3])
  end
end

local function read_angle(degrees)
  return degrees / 180 * dort.math.pi
end

local function read_transform(json)
  local id = dort.geometry.identity()
  local translate = id
  local scale = id
  local rotate = id
  local look_at = id

  if json["look_at"] then
    look_at = dort.geometry.look_at(
      read_point(json["position"]),
      read_point(json["look_at"]),
      read_vector(json["up"]))
  elseif json["position"] then
    translate = dort.geometry.translate(read_vector(json["position"]))
  end
  if json["scale"] then
    if type(json["scale"]) == "number" then
      scale = dort.geometry.scale(json["scale"])
    else
      scale = dort.geometry.scale(json["scale"][1], json["scale"][2], json["scale"][3])
    end
  end
  if json["rotation"] then
    rotate = 
      dort.geometry.rotate_x(read_angle(json["rotation"][1])) *
      dort.geometry.rotate_y(read_angle(json["rotation"][2])) *
      dort.geometry.rotate_z(read_angle(json["rotation"][3]))
  end

  return translate * look_at * scale * rotate
end

local read_texture
local texture_types = {
  checker = function(json, input, output)
    if input ~= "geom" then
      error("Unimplemented checker input type")
    end

    local map = dort.texture.make_map_uv()
    local checker = dort.texture.make_checkerboard_2d {
      even = read_texture(json["on_color"], "2d", output),
      odd = read_texture(json["off_color"], "2d", output),
      check_size = 1/json["res_u"],
    }
    return checker .. map
  end,
}

function read_texture(json, input, output)
  if type(json) == "table" and json["type"] then
    if not texture_types[json["type"]] then
      error("Unknown texture type: " .. json["type"])
    end
    return texture_types[json["type"]](json, input, output)
  end

  local const_value
  if output == "spectrum" then
    const_value = read_spectrum(json)
  elseif output == "float" then
    const_value = json
  else
    error(output)
  end

  if input == "geom" then
    return dort.texture.make_const_geom(const_value)
  elseif input == "1d" then
    return dort.texture.make_const_1d(const_value)
  elseif input == "2d" then
    return dort.texture.make_const_2d(const_value)
  elseif input == "3d" then
    return dort.texture.make_const_3d(const_value)
  else
    error(input)
  end
end


local bsdf_types = {
  lambert = function(ctx, json)
    return dort.material.make_lambert {
      albedo = read_texture(json["albedo"], "geom", "spectrum"),
    }
  end,
  dielectric = function(ctx, json)
    if json["enable_refraction"] then
      error("glass not implemented")
    else
      return dort.material.make_mirror {
        albedo = read_texture(json["albedo"], "geom", "spectrum"),
      }
    end
  end,
  rough_conductor = function(ctx, json)
    -- TODO
    return dort.material.make_lambert {
      albedo = read_texture(json["albedo"], "geom", "spectrum"),
    }
  end,
  smooth_coat = function(ctx, json)
    -- TODO
    return ctx.bsdfs[json["substrate"]]
  end,
  null = function(ctx, json)
    -- TODO
    return dort.material.make_lambert { albedo = dort.spectrum.rgb(0) }
  end,
}

local function read_bsdf(ctx, json)
  if type(json) == "string" then
    return ctx.bsdfs[json]
  end
  if not bsdf_types[json["type"]] then
    error("unknown bsdf type: " .. json["type"])
  end

  local material = bsdf_types[json["type"]](ctx, json)
  if json["bump"] then
    local bump_path = ctx.scene_dir .. "/" .. json["bump"]
    local bump_image = dort.image.read(bump_path, { hdr = false })
    local bump_texture = dort.texture.make_image(bump_image)
    material = dort.material.make_bump {
      bump = dort.texture.make_average()
        .. bump_texture .. dort.texture.make_map_uv(),
      material = material,
    }
  end
  return material
end


local primitive_types = {
  mesh = function(ctx, json)
    --if json["smooth"] then error("smooth mesh not implemented") end
    --if json["backface_culling"] then error("mesh backface_culling not implemented") end
    --if json["recompute_normals"] then error("mesh recompute_normals not implemented") end
    if json["power"] or json["emission"] then
      error("mesh emission not implemented")
    end

    local wo3_file = io.open(ctx.scene_dir .. "/" .. json["file"], "r")

    local vertex_count = string.unpack("I8", wo3_file:read(8))
    local points = {}
    local uvs = {}
    local normals = nil
    if json["smooth"] then normals = {} end

    for i = 1, vertex_count do
      local x, y, z, nx, ny, nz, u, v = string.unpack("fff fff ff", wo3_file:read(32))
      points[i] = dort.geometry.point(x, y, z)
      uvs[i] = dort.geometry.vec2(u, v)
      if json["smooth"] then
        normals[i] = dort.geometry.normal(nx, ny, nz)
      end
    end

    local triangle_count = string.unpack("I8", wo3_file:read(8))
    local vertices = {}
    local triangle_idxs = {}
    for i = 1, triangle_count do
      local v1, v2, v3, mat = string.unpack("I4I4I4 i4", wo3_file:read(16))
      vertices[3*i - 2] = v1
      vertices[3*i - 1] = v2
      vertices[3*i] = v3
      triangle_idxs[i] = 3*(i - 1)
    end

    wo3_file:close()

    if json["recompute_normals"] then
      error("mesh recompute_normals not yet implemented")
    end
    
    local mesh = dort.shape.make_mesh {
      transform = dort.builder.get_transform(ctx.b),
      points = points,
      uvs = uvs,
      normals = normals,
      vertices = vertices,
    }

    for i = 1, triangle_count do
      dort.builder.add_triangle(ctx.b, mesh, triangle_idxs[i])
    end
  end,
  quad = function(ctx, json)
    local mesh = dort.shape.make_mesh {
      transform = dort.builder.get_transform(ctx.b),
      points = {
        dort.geometry.point(-0.5, 0, -0.5),
        dort.geometry.point( 0.5, 0, -0.5),
        dort.geometry.point( 0.5, 0,  0.5),
        dort.geometry.point(-0.5, 0,  0.5),
      },
      uvs = {
        dort.geometry.vec2(0, 0),
        dort.geometry.vec2(1, 0),
        dort.geometry.vec2(1, 1),
        dort.geometry.vec2(0, 1),
      },
      vertices = {
        0, 1, 2,
        0, 2, 3,
      }
    }

    dort.builder.add_triangle(ctx.b, mesh, 0)
    dort.builder.add_triangle(ctx.b, mesh, 3)

    if json["power"] or json["emission"] then
      local tri_1 = dort.builder.make_triangle(ctx.b, mesh, 0)
      local tri_2 = dort.builder.make_triangle(ctx.b, mesh, 3)
      local emission
      if json["power"] then
        local area = dort.shape.get_area(tri_1) + dort.shape.get_area(tri_2)
        emission = read_spectrum(json["power"]) / area
      else
        emission = read_spectrum(json["emission"])
      end

      dort.builder.add_light(ctx.b, dort.light.make_diffuse {
        shape = tri_1, radiance = emission,
      })
      dort.builder.add_light(ctx.b, dort.light.make_diffuse {
        shape = tri_2, radiance = emission,
      })
    end
  end,
  infinite_sphere = function(ctx, json)
    if not json["emission"] then
      error("primitive inifinite_sphere without emission")
    end
    local image_path = ctx.scene_dir .. "/" .. json["emission"]
    local image = dort.image.read(image_path, { hdr = true })
    dort.builder.add_light(ctx.b, dort.light.make_environment {
      image = image,
      up = dort.geometry.vector(0, 1, 0),
      forward = dort.geometry.vector(1, 0, 0),
      scale = dort.spectrum.rgb(1),
      transform = dort.builder.get_transform(ctx.b),
    })
  end,
}

local function read_primitive(ctx, json)
  if not primitive_types[json["type"]] then
    error("unknown primitive type: " .. json["type"])
  end

  dort.builder.push_state(ctx.b)
  dort.builder.set_material(ctx.b, read_bsdf(ctx, json["bsdf"]))
  dort.builder.set_transform(ctx.b, read_transform(json["transform"]))
  primitive_types[json["type"]](ctx, json)
  dort.builder.pop_state(ctx.b)
end


local camera_types = {
  pinhole = function(json)
    return dort.camera.make_pinhole {
      transform = read_transform(json["transform"])
        * dort.geometry.scale(-1, -1, 1),
      fov = read_angle(json["fov"]),
    }
  end,
}

local function read_camera(json)
  if not camera_types[json["type"]] then
    error("unknown camera type: " .. json["type"])
  end
  return camera_types[json["type"]](json)
end



local function read(scene_dir, opts)
  local opts = opts or {}
  local scene_file = io.open(scene_dir .. "/scene.json", "r")
  local scene_text = scene_file:read("a")
  if not scene_text then
    error("Could not read scene file")
  end

  local scene_json, idx, err = json.decode(scene_text)
  scene_file:close()
  if not scene_json then
    error("Could not decode scene JSON: " .. err)
  end

  local b = dort.builder.make()
  local ctx = {
    b = b,
    bsdfs = {},
    scene_dir = scene_dir,
  }

  if opts.bsdfs then
    for name, bsdf in pairs(opts.bsdfs) do
      ctx.bsdfs[name] = bsdf
    end
  end

  for _, bsdf_json in ipairs(scene_json["bsdfs"]) do
    ctx.bsdfs[bsdf_json["name"]] = read_bsdf(ctx, bsdf_json)
  end

  for _, prim_json in ipairs(scene_json["primitives"]) do
    read_primitive(ctx, prim_json)
  end

  dort.builder.set_camera(b, read_camera(scene_json["camera"]))
  return dort.builder.build_scene(b)
end



return {
  read = read,
}
