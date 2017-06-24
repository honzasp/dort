--- Lua utilities
-- @module dort.std
dort.std = {}

--- Print to stdout using @{string.format}.
-- @function printf
-- @param format
-- @param ...
function dort.std.printf(format, ...)
  return io.write(string.format(format, ...))
end

--- Create a shallow copy of a table.
-- @function clone
-- @param table
function dort.std.clone(table)
  local clone = {}
  for key, value in pairs(table) do
    clone[key] = value
  end
  return clone
end

--- Create a new table with values from both `table1` and `table2`.
-- If both `table1` and `table2` contain the same key, the value from `table2`
-- will be used.
-- @function merge
-- @param table1
-- @param table2
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
