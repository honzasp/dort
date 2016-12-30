local _ENV = require 'dort/dsl'
local lgi = require 'lgi'
local Gtk = lgi.Gtk
local GdkPixbuf = lgi.GdkPixbuf
local GLib = lgi.GLib

local scene = define_scene(function()
  local white = matte_material { color = rgb(0.5, 0.5, 0.5) }
  local green = matte_material { color = rgb(0, 0.5, 0) }
  local red = matte_material { color = rgb(0.5, 0, 0) }

  local right_box = mirror_material { color = rgb(1, 1, 1) }
  local left_box = glass_material { color = rgb(1, 1, 1) }

  block(function()
    local m = mesh {
      points = {
        point(552.8, 0, 0),
        point(0, 0, 0),
        point(0, 0, 559.2),
        point(549.6, 0, 559.2),

        point(556, 548.8, 0),
        point(556, 548.8, 559.2),
        point(0, 548.8, 559.2),
        point(0, 548.8, 0),
      },
      vertices = {
        0, 1, 2,  0, 2, 3,
        4, 5, 6,  4, 6, 7,
        6, 5, 3,  6, 3, 2,
        7, 6, 2,  7, 2, 1,
        0, 3, 5,  0, 5, 4,
      },
    }

    material(white)
    add_triangle(m, 0)
    add_triangle(m, 3)
    add_triangle(m, 6)
    add_triangle(m, 9)
    add_triangle(m, 12)
    add_triangle(m, 15)

    material(green)
    add_triangle(m, 18)
    add_triangle(m, 21)

    material(red)
    add_triangle(m, 24)
    add_triangle(m, 27)
  end)

  block(function()
    material(left_box)
    transform(
        translate(185, 82.5, 169)
      * rotate_y(-0.29) 
      * scale(165 / 2))
    add_shape(cube())
  end)

  block(function()
    material(right_box)
    transform(
        translate(368, 165, 351) 
      * rotate_y(-1.27) 
      * scale(165 / 2, 330 / 2, 165 / 2))
    add_shape(cube())
  end)

  camera(pinhole_camera {
    transform = look_at(
      point(278, 273, -800),
      point(278, 273, 0),
      vector(0, 1, 0)) * scale(1, -1, 1),
    fov = 0.686,
  })

  block(function() 
    transform(translate(0, -1, 0))
    local m = mesh {
      points = {
        point(343, 548.8, 227),
        point(343, 548.8, 332),
        point(213, 548.8, 332),
        point(213, 548.8, 227),
      },
      vertices = {
        2, 1, 0,  3, 2, 0,
      },
    }
    for _, index in ipairs({0, 3}) do
      add_light(diffuse_light {
        radiance = rgb(1, 1, 1) * 70,
        shape = triangle(m, index),
        num_samples = 8,
      })
    end
  end)
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
  },
})
window:show_all()

local render_job = nil
local is_cancelling = false
window.child.width_entry.value = 200
window.child.height_entry.value = 200
window.child.iterations_entry.value = 100
window.child.light_paths_entry.value = 10*1000

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
    renderer = "sppm",
    initial_radius = 20,
    iterations = iterations,
    light_paths = light_paths,
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

function window:on_destroy()
  cancel_render()
  Gtk.main_quit()
end

update()
GLib.timeout_add_seconds(GLib.PRIORITY_DEFAULT, 2, refresh)
Gtk.main()
