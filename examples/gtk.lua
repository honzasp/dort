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

local vbox = Gtk.VBox()
vbox:add(Gtk.Button {
  label = "Start the computation",
  on_clicked = function()
    print("starting the computation")
    dort.builder.render_in_background(function(answer)
      print("the computation finished: " .. answer)
    end)
  end,
})
window:add(vbox)

window:show_all()
print("entering main")
Gtk.main()
print("leaving main")
