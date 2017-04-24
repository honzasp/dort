require "dort.std"

local COLORS = {
  red = "\x1b[31m",
  green = "\x1b[32m",
  yellow = "\x1b[33m",
  cyan = "\x1b[36m",
  reset = "\x1b[0m",
}

local t = {
  dir = "test",
  pattern = {"simple", "mixd"},
  renderer = nil,
  compare_with_ref = true,
  generate_ref = false,
  tests = {},

  test = function(self, name, scene, render_optss, ref_opts)
    self.tests[#self.tests + 1] = {name, scene, render_optss, ref_opts}
  end,

  load_tests = function(self, file)
    (dofile(self.dir .. "/" .. file))(self)
  end,

  run = function(self)
    local count = 0
    local errs = 0
    local time_s = 0
    for _, test in ipairs(self.tests) do
      local test_count, test_err, test_s = self:run_test(
        test[1], test[2], test[3], test[4])
      count = count + test_count
      errs = errs + test_err
      time_s = time_s + test_s
    end

    if errs == 0 then
      dort.std.printf("%s%d tests passed (%g s)%s all bugs hidden :-(%s\n",
        COLORS.green, count, time_s, COLORS.cyan, COLORS.reset)
    else
      dort.std.printf("%s%d/%d tests failed (%g s)%s no problem :-)%s\n",
        COLORS.red, errs, count, time_s, COLORS.green, COLORS.reset)
    end
  end,

  run_test = function(self, name, scene, render_optss, ref_opts)
    if self.pattern then
      for _, pattern in ipairs(self.pattern) do
        if not string.find(name, pattern) then
          return 0, 0, 0
        end
      end
    end

    dort.std.printf("test %s\n", name)
    local ref_path = string.format("%s/ref/%s.hdr", self.dir, name)
    local out_dir_path = string.format("%s/out/%s", self.dir, name)

    if self.generate_ref and ref_opts then
      dort.std.printf("  generating ref (%s)...", ref_opts.renderer)
      io.stdout:flush()
      local out_image, ref_time_s = self:render(scene, ref_opts)
      dort.image.write_rgbe(ref_path, out_image)
      dort.std.printf(" %sdone (%g s)%s\n", COLORS.cyan, ref_time_s, COLORS.reset)
    end

    if self.generate_ref then
      return 0, 0, 0
    end

    -- TODO: nasty hack
    os.execute(string.format("mkdir -p %q", out_dir_path))
    local ref_image
    if self.compare_with_ref then
      ref_image = dort.image.read(ref_path, { hdr = true })
      dort.image.write_rgbe(out_dir_path .. "/ref.hdr", ref_image)
    end

    local count = 0
    local errs = 0
    local time_s = 0
    for _, render_opts in ipairs(render_optss) do
      if self.renderer and render_opts.renderer ~= self.renderer then
        goto next_iter
      end

      dort.std.printf("  %s...", render_opts.renderer)
      io.stdout:flush()
      local out_image, test_time_s = self:render(scene, render_opts)
      dort.image.write_rgbe(string.format("%s/%s.hdr",
        out_dir_path, render_opts.renderer), out_image)

      if self.compare_with_ref then
        local x_res, y_res = dort.image.get_res(ref_image)
        local bias, variance = dort.image.bias_variance(ref_image, out_image,
          dort.geometry.recti(0, 0, x_res, y_res))
        local norm_bias = bias / (3 * x_res * y_res)
        local norm_sd = dort.math.sqrt(variance / (3 * x_res * y_res))

        local status
        if dort.math.abs(norm_bias) < 0.05 then
          if norm_sd < 1.5 then
            status = "ok"
          else
            status = "warn"
          end
        else
          status = "err"
        end

        if status == "err" then errs = errs + 1 end

        local color = ({
          ok = COLORS.green,
          warn = COLORS.yellow,
          err = COLORS.red,
        })[status]
        dort.std.printf(" %s%s (%g s): %g, %g%s\n", color, status, test_time_s,
          norm_bias, norm_sd, COLORS.reset)
      else
        dort.std.printf(" %sdone (%g s)%s\n", COLORS.cyan, test_time_s, COLORS.reset)
      end

      count = count + 1
      time_s = time_s + test_time_s
      ::next_iter::
    end

    return count, errs, time_s
  end,

  render = function(self, scene, render_opts)
    local render = dort.render.make(scene, dort.std.clone(render_opts))
    local time_begin = dort.chrono.now()
    dort.render.render_sync(render)
    local time_end = dort.chrono.now()
    local out_image = dort.render.get_image(render, { hdr = true })
    local time_s = dort.chrono.difference_s(time_begin, time_end)
    return out_image, time_s
  end,
}

t:load_tests("test_simple.lua")
t:load_tests("test_box.lua")
t:run()
