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

function dort.std.merge(table1, table2)
  local clone = {}
  for key, value in pairs(table1) do
    clone[key] = value
  end
  for key, value in pairs(table2) do
    clone[key] = value
  end
  return clone
end

return dort.std
