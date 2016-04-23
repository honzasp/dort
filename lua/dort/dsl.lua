require "dort/helpers"

local dsl = {}

for k,v in pairs(_G) do
  dsl[k] = v
end

for k,v in pairs(dort.math) do
  dsl[k] = v
end

local b = dort.builder
local B

function dsl.get_builder()
  return B
end
function dsl.get_transform()
  return b.get_transform(B)
end

function dsl.define_scene(callback)
  local prev_builder = B
  local builder = b.make()

  B = builder
  callback(builder)

  local scene = b.build_scene(builder)
  B = prev_builder
  return scene
end

function dsl.block(callback) return b.block(B, callback) end
function dsl.frame(callback) return b.frame(B, callback) end

function dsl.transform(transform) b.set_transform(B, transform) end
function dsl.material(material) b.set_material(B, material) end
function dsl.camera(camera) b.set_camera(B, camera) end
function dsl.option(opt, ...) b.set_option(B, opt, table.unpack(arg)) end

function dsl.add_shape(shape) b.add_shape(B, shape) end
function dsl.add_primitive(prim) b.add_primitive(B, prim) end
function dsl.add_triangle(mesh, index) b.add_triangle(B, mesh, index) end
function dsl.add_light(light) b.add_light(B, light) end
function dsl.add_read_ply_mesh(file_name) b.add_read_ply_mesh(B, file_name) end
function dsl.add_read_ply_mesh_as_bvh(file_name)
  b.add_read_ply_mesh_as_bvh(B, file_name)
end
function dsl.add_ply_mesh(mesh) b.add_ply_mesh(B, mesh) end
function dsl.add_ply_mesh_as_bvh(mesh) b.add_ply_mesh_as_bvh(B, mesh) end
function dsl.add_voxel_grid(params) b.add_voxel_grid(B, params) end

dsl.render = dort.builder.render

dsl.ortho_camera = dort.camera.make_ortho
dsl.perspective_camera = dort.camera.make_perspective

dsl.box_filter = dort.filter.make_box
dsl.triangle_filter = dort.filter.make_triangle
dsl.gaussian_filter = dort.filter.make_gaussian
dsl.mitchell_filter = dort.filter.make_mitchell
dsl.lanczos_sinc_filter = dort.filter.make_lanczos_sinc

dsl.vector = dort.geometry.vector
dsl.point = dort.geometry.point
dsl.identity = dort.geometry.identity
dsl.inverse = dort.geometry.inverse
dsl.translate = dort.geometry.translate
dsl.scale = dort.geometry.scale
dsl.rotate_x = dort.geometry.rotate_x
dsl.rotate_y = dort.geometry.rotate_y
dsl.rotate_z = dort.geometry.rotate_z
dsl.rotate_x_around = dort.geometry.rotate_x_around
dsl.rotate_y_around = dort.geometry.rotate_y_around
dsl.rotate_z_around = dort.geometry.rotate_z_around
dsl.look_at = dort.geometry.look_at
dsl.stretch = dort.geometry.stretch
dsl.vec3i = dort.geometry.vec3i
dsl.vec2 = dort.geometry.vec2
dsl.boxi = dort.geometry.boxi

dsl.read_image = dort.image.read
dsl.write_png_image = dort.image.write_png

function apply_builder_transform(transform)
  local builder_transform = b.get_transform(B)
  if transform then
    return builder_transform * transform
  else
    return builder_transform
  end
end

function dsl.point_light(params)
  params.transform = apply_builder_transform(params.transform)
  return dort.light.make_point(params)
end
function dsl.diffuse_light(params)
  params.transform = apply_builder_transform(params.transform)
  return dort.light.make_diffuse(params)
end
dsl.infinite_light = dort.light.make_infinite

dsl.matte_material = dort.material.make_matte
dsl.plastic_material = dort.material.make_plastic
dsl.metal_material = dort.material.make_metal
dsl.mirror_material = dort.material.make_mirror
dsl.glass_material = dort.material.make_glass
dsl.rough_glass_material = dort.material.make_rough_glass
dsl.bump_material = dort.material.make_bump

dsl.random_sampler = dort.sampler.make_random
dsl.stratified_sampler = dort.sampler.make_stratified

dsl.sphere = dort.shape.make_sphere
dsl.disk = dort.shape.make_disk
dsl.cube = dort.shape.make_cube
dsl.polygon = dort.shape.make_polygon
dsl.read_ply_mesh = dort.shape.read_ply_mesh

function dsl.mesh(params)
  if not params.transform then
    params.transform = b.get_transform(B)
  end
  return dort.shape.make_mesh(params)
end

dsl.rgb = dort.spectrum.rgb
dsl.rgbh = dort.spectrum.rgbh

dsl.reset_stats = dort.stats.reset
dsl.write_and_reset_stats = dort.stats.write_and_reset

dsl.compose_texture = dort.texture.compose
dsl.const_texture = dort.texture.make_const_geom
dsl.const_texture_1d = dort.texture.make_const_1d
dsl.const_texture_2d = dort.texture.make_const_2d
dsl.const_texture_3d = dort.texture.make_const_3d
dsl.identity_texture_1d = dort.texture.make_identity_1d
dsl.identity_texture_2d = dort.texture.make_identity_2d
dsl.identity_texture_3d = dort.texture.make_identity_3d
dsl.lerp_texture = dort.texture.make_lerp
dsl.checkerboard_texture_1d = dort.texture.make_checkerboard_1d
dsl.checkerboard_texture_2d = dort.texture.make_checkerboard_2d
dsl.checkerboard_texture_3d = dort.texture.make_checkerboard_3d
dsl.value_noise_texture_1d = dort.texture.make_value_noise_1d
dsl.value_noise_texture_2d = dort.texture.make_value_noise_2d
dsl.value_noise_texture_3d = dort.texture.make_value_noise_3d
dsl.value_noise_texture_1d_of_2d = dort.texture.make_value_noise_1d_of_2d
dsl.value_noise_texture_2d_of_2d = dort.texture.make_value_noise_2d_of_2d
dsl.value_noise_texture_3d_of_2d = dort.texture.make_value_noise_3d_of_2d
dsl.value_noise_texture_1d_of_3d = dort.texture.make_value_noise_1d_of_3d
dsl.value_noise_texture_2d_of_3d = dort.texture.make_value_noise_2d_of_3d
dsl.value_noise_texture_3d_of_3d = dort.texture.make_value_noise_3d_of_3d
dsl.image_texture = dort.texture.make_image
dsl.gain_texture = dort.texture.make_gain
dsl.bias_texture = dort.texture.make_bias
dsl.uv_texture_map = dort.texture.make_map_uv
dsl.xy_texture_map = dort.texture.make_map_xy
dsl.spherical_texture_map = dort.texture.make_map_spherical
dsl.cylindrical_texture_map = dort.texture.make_map_cylindrical
dsl.xyz_texture_map = dort.texture.make_map_xyz
dsl.grayscale_color_map = dort.texture.make_color_map_grayscale
dsl.lerp_color_map = dort.texture.make_color_map_lerp
dsl.spline_color_map = dort.texture.make_color_map_spline
dsl.render_texture_2d = dort.texture.render_2d

dort.dsl = dsl
return dsl
