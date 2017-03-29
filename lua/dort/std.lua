dort.std = {}

function dort.std.printf(format, ...)
  return io.write(string.format(format, ...))
end

function dort.std.clone(table)
  local clone = {}
  for key, value in pairs(table) do
    clone[key] = value
  end
  return clone
end

return dort.std
