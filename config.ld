-- configuration file for LDoc
-- (this file is a Lua program)
-- generate the docs with `ldoc .`

project = "dort"
title = "dort API"
description = "Lua API of the dort renderer"
style = "!pale"
no_return_or_parms = true
sort = true

file = {"src/dort", "lua/dort"}
add_language_extension("cpp", "c")
dir = ".ldoc"
merge = true
format = "markdown"
backtick_references = false
