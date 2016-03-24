local dsl = {}

for k,v in pairs(_G) do
  dsl[k] = v
end

for k,v in pairs(dort.math) do
  dsl[k] = v
end

dsl.define_scene = dort.builder.define_scene
dsl.block = dort.builder.define_block
dsl.instance = dort.builder.define_instance
dsl.transform = dort.builder.set_transform
dsl.material = dort.builder.set_material
dsl.camera = dort.builder.set_camera
dsl.option = dort.builder.set_option
dsl.add_shape = dort.builder.add_shape
dsl.add_primitive = dort.builder.add_primitive
dsl.add_light = dort.builder.add_light
dsl.add_read_ply_mesh = dort.builder.add_read_ply_mesh
dsl.add_read_ply_mesh_as_bvh = dort.builder.add_read_ply_mesh_as_bvh
dsl.add_ply_mesh = dort.builder.add_ply_mesh
dsl.add_ply_mesh_as_bvh = dort.builder.add_ply_mesh_as_bvh
dsl.add_voxel_grid = dort.builder.add_voxel_grid
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
dsl.translate = dort.geometry.translate
dsl.scale = dort.geometry.scale
dsl.rotate_x = dort.geometry.rotate_x
dsl.rotate_y = dort.geometry.rotate_y
dsl.rotate_z = dort.geometry.rotate_z
dsl.look_at = dort.geometry.look_at
dsl.vec3i = dort.geometry.vec3i
dsl.boxi = dort.geometry.boxi

dsl.read_image = dort.image.read
dsl.write_png_image = dort.image.write_png

dsl.point_light = dort.light.make_point
dsl.diffuse_light = dort.light.make_diffuse
dsl.infinite_light = dort.light.make_infinite

dsl.matte_material = dort.material.make_matte
dsl.plastic_material = dort.material.make_plastic
dsl.metal_material = dort.material.make_metal
dsl.mirror_material = dort.material.make_mirror
dsl.glass_material = dort.material.make_glass
dsl.rough_glass_material = dort.material.make_rough_glass

dsl.random_sampler = dort.sampler.make_random
dsl.stratified_sampler = dort.sampler.make_stratified

dsl.sphere = dort.shape.make_sphere
dsl.disk = dort.shape.make_disk
dsl.triangle = dort.shape.make_triangle
dsl.mesh = dort.shape.make_mesh
dsl.read_ply_mesh = dort.shape.read_ply_mesh

dsl.rgb = dort.spectrum.rgb

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
dsl.render_texture_2d = dort.texture.render_2d

dort.dsl = dsl
return dsl
