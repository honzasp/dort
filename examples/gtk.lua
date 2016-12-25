local lgi = require 'lgi'
local Gtk = lgi.Gtk

local window = Gtk.Window {
  title = "Window",
  default_width = 400,
  default_height = 300,
  on_destroy = Gtk.main_quit,
}

if tonumber(Gtk._version) >= 3 then
  window.has_resize_grip = true
end

window:show_all()
print("entering main")
Gtk.main()
print("leaving main")
