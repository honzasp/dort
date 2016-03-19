local _env = {}
for k, v in pairs(_G) do
  _env[k] = v
end

_env.b = minecraft.blocks
_env.m = dort.material
_env.rgb = dort.spectrum.rgb
return _env
