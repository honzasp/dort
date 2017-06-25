-- configuration file for LDoc
-- (this file is a Lua program)
-- generate the docs with `ldoc .`

project = "dort"
title = "dort API"
description = "Lua API of the `dort` renderer"
style = "!pale"
no_return_or_parms = true
--sort = true

file = {"src/dort", "lua/dort"}
add_language_extension("cpp", "c")
dir = ".ldoc"
merge = true
format = "markdown"
backtick_references = false

full_description = [[
The API surface is quite large, so the defined functions are separated into
many modules. The most important modules are:

- @{dort.render} -- rendering scenes
- @{dort.builder} -- building scenes to render

Rendering also requires some objects defined in:

- @{dort.camera} -- `Camera`
- @{dort.sampler} -- `Sampler`
- @{dort.filter} -- `Filter` (for image reconstruction)

while scenes are built from:

- @{dort.shape} -- `Shape` and `Mesh`
- @{dort.light} -- `Light`
- @{dort.material} -- `Material`

using (among others)

- @{dort.geometry} -- vector types, `Transform`
- @{dort.texture} -- `Texture`
- @{dort.spectrum} -- `Spectrum` (color)
- @{dort.image} -- `Image.Rgb8` (LDR image) and `Image.RgbFloat` (HDR image)

**Warning:** all functions that take as argument a table with named parameters
(usually named `params`) _may mutate the table_.
]]
