local _ENV = require 'dort/dsl'
local lgi = require 'lgi'
local Gtk = lgi.Gtk
local GdkPixbuf = lgi.GdkPixbuf
local GLib = lgi.GLib
local minecraft = require "minecraft"

local scene = define_scene(function()
  minecraft.add_world(get_builder(), {
    map = os.getenv("HOME") .. "/.minecraft/saves/Specular",
    box = boxi(vec3i(-20, 0, -20), vec3i(20, 20, 20)),
  })
  --[[
  add_light(infinite_light {
    radiance = rgb(0.2),
  })
  --]]
  ----[[
  for z = -20, 20, 10 do
    for x = -20, 20, 10 do
      add_light(point_light {
        point = point(x, 20, z),
        intensity = rgb(60),
      })
    end
  end
  --]]
end)

local window = Gtk.Window {
  title = "Window",
  default_width = 400,
  default_height = 300,
}

window:add(Gtk.Box {
  orientation = "HORIZONTAL",
  Gtk.Image {
    id = "preview_image",
    margin_bottom = 0,
    margin_top = 0,
    margin_left = 0,
    margin_right = 0,
    hexpand = true,
    vexpand = true,
    width = 300,
  },
  Gtk.Box {
    orientation = "VERTICAL",
    width = 200,
    Gtk.Button {
      id = "render_button",
      label = "",
    },
    Gtk.ProgressBar {
      id = "progress_bar",
      visible = false,
      show_text = true,
    },
    Gtk.SpinButton {
      id = "width_entry",
      adjustment = Gtk.Adjustment { lower = 1, upper = 10000, step_increment = 10 },
    },
    Gtk.SpinButton {
      id = "height_entry",
      adjustment = Gtk.Adjustment { lower = 1, upper = 10000, step_increment = 10 },
    },
    Gtk.SpinButton {
      id = "iterations_entry",
      adjustment = Gtk.Adjustment { lower = 1, upper = 10000, step_increment = 1 },
    },
    Gtk.SpinButton {
      id = "light_paths_entry",
      adjustment = Gtk.Adjustment {
        lower = 1000,
        upper = 100*1000*1000,
        step_increment = 1000,
      },
    },
    Gtk.Grid {
      { Gtk.Button { id = "roll_left_button", label = "Roll left" },
        left_attach = 0, top_attach = 0 },
      { Gtk.Button { id = "roll_right_button", label = "Roll right" },
        left_attach = 2, top_attach = 0 },
      { Gtk.Button { id = "pitch_up_button", label = "Pitch up" },
        left_attach = 1, top_attach = 0 },
      { Gtk.Button { id = "pitch_down_button", label = "Pitch down" },
        left_attach = 1, top_attach = 1 },
      { Gtk.Button { id = "yaw_left_button", label = "Yaw left" },
        left_attach = 0, top_attach = 1 },
      { Gtk.Button { id = "yaw_right_button", label = "Yaw right" },
        left_attach = 2, top_attach = 1 },
    },
    Gtk.Grid {
      { Gtk.Button { id = "move_fwd_button", label = "Forward" },
        left_attach = 0, top_attach = 0 },
      { Gtk.Button { id = "move_bwd_button", label = "Backward" },
        left_attach = 2, top_attach = 0 },
      { Gtk.Button { id = "move_up_button", label = "Up" },
        left_attach = 1, top_attach = 0 },
      { Gtk.Button { id = "move_down_button", label = "Down" },
        left_attach = 1, top_attach = 1 },
      { Gtk.Button { id = "move_left_button", label = "Left" },
        left_attach = 0, top_attach = 1 },
      { Gtk.Button { id = "move_right_button", label = "Right" },
        left_attach = 2, top_attach = 1 },
    },
  },
})
window:show_all()

local render_job = nil
local is_cancelling = false
window.child.width_entry.value = 600
window.child.height_entry.value = 400
window.child.iterations_entry.value = 100
window.child.light_paths_entry.value = 100*1000

local camera_transform = look_at(
      point(-10, 12, -11),
      point(3, 3, 4),
      vector(0, 1, 0)
  ) * scale(-1, -1, 1)

function start_render()
  if is_cancelling then
    return
  end

  local int = math.tointeger
  local width = int(max(1, window.child.width_entry.value))
  local height = int(max(1, window.child.height_entry.value))
  local iterations = int(window.child.iterations_entry.value)
  local light_paths = int(window.child.light_paths_entry.value)

  is_cancelling = false
  render_job = dort.render.make(scene, {
    x_res = width, y_res = height,
    max_depth = 10,
    sampler = stratified_sampler {
      samples_per_x = 1,
      samples_per_y = 1,
    },
    filter = mitchell_filter {
      radius = 1.5,
    },
    camera = pinhole_camera {
      transform = camera_transform,
      fov = 0.686,
    },
    renderer = "bdpt",
    iterations = iterations,
    use_t1_paths = true,
    --initial_radius = 1,
    --light_paths = light_paths,
  })

  dort.render.render_async(render_job, finish_render)
  update()
end

function cancel_render()
  if is_cancelling or render_job == nil then
    return
  end

  is_cancelling = true
  dort.render.cancel(render_job)
  update()
end

function finish_render()
  if not is_cancelling then
    local image = dort.render.get_image(render_job, { hdr = false })
    dort.image.write_png("gtk_output.png", image)
    preview_image(image)
  end

  render_job = nil
  is_cancelling = false
  update()
end

function preview_image(image)
  local pixbuf = GdkPixbuf.Pixbuf(dort.render.image_to_pixbuf(image))
  window.child.preview_image:set_from_pixbuf(pixbuf)
end

function refresh()
  if render_job then
    preview_image(dort.render.get_preview(render_job))
  end
  update()
  return true
end

function update()
  local bar = window.child.progress_bar
  local button = window.child.render_button
  if render_job then
    bar:show()
    bar.fraction = dort.render.get_progress(render_job)
    if is_cancelling then
      bar.text = "Cancelling..."
    else
      bar.text = "Rendering..."
    end
    button.label = "Cancel"
  else
    bar:hide()
    button.label = "Render"
  end
end

function window.child.render_button:on_clicked()
  if render_job == nil then
    start_render()
  else
    cancel_render()
  end
end

function update_camera(transform)
  camera_transform = camera_transform * transform
end

function window.child.roll_left_button:on_clicked()
  update_camera(dort.geometry.rotate_z(-pi/30))
end
function window.child.roll_right_button:on_clicked()
  update_camera(dort.geometry.rotate_z(pi/30))
end
function window.child.pitch_up_button:on_clicked()
  update_camera(dort.geometry.rotate_x(pi/30))
end
function window.child.pitch_down_button:on_clicked()
  update_camera(dort.geometry.rotate_x(-pi/30))
end
function window.child.yaw_left_button:on_clicked()
  update_camera(dort.geometry.rotate_y(-pi/30))
end
function window.child.yaw_right_button:on_clicked()
  update_camera(dort.geometry.rotate_y(pi/30))
end

function translate_camera(vec)
  update_camera(dort.geometry.translate(vec))
end

local delta = 1
function window.child.move_up_button:on_clicked()
  translate_camera(dort.geometry.vector(0, -delta, 0))
end
function window.child.move_down_button:on_clicked()
  translate_camera(dort.geometry.vector(0, delta, 0))
end
function window.child.move_left_button:on_clicked()
  translate_camera(dort.geometry.vector(-delta, 0, 0))
end
function window.child.move_right_button:on_clicked()
  translate_camera(dort.geometry.vector(delta, 0, 0))
end
function window.child.move_fwd_button:on_clicked()
  translate_camera(dort.geometry.vector(0, 0, delta))
end
function window.child.move_bwd_button:on_clicked()
  translate_camera(dort.geometry.vector(0, 0, -delta))
end


function window:on_destroy()
  cancel_render()
  Gtk.main_quit()
end

update()
GLib.timeout_add(GLib.PRIORITY_DEFAULT, 500, refresh)
Gtk.main()
