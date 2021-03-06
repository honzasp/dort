local _env = {}

for k, v in pairs(_G) do
  _env[k] = v
end

for k, v in pairs(dort.math) do
  _env[k] = v
end

_env.b = dort.builder
_env.m = dort.material
_env.t = dort.texture
_env.g = dort.geometry
_env.rgb = dort.spectrum.rgb
_env.rgbh = dort.spectrum.rgbh
_env.s = dort.shape
_env.l = dort.light
return _env
