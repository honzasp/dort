require "dort.std"

local COLORS = {
  red = "\x1b[31m",
  green = "\x1b[32m",
  yellow = "\x1b[33m",
  cyan = "\x1b[36m",
  reset = "\x1b[0m",
}

local dir = "test"

local opts = {
  pattern = {},
  render = nil,
  compare_with_ref = true,
  generate_ref = false,
  verbose = false,
}

local function parse_argv(argv)
  local i = 1
  while i <= #argv do
    local arg = argv[i]
    if arg == "-r" or arg == "--render" then
      opts.render = argv[i+1]
      i = i + 1
    elseif arg == "-n" or arg == "--no-ref" then
      opts.compare_with_ref = false
    elseif arg == "-g" or arg == "--generate-ref" then
      opts.generate_ref = true
    elseif arg == "-v" or arg == "--verbose" then
      opts.verbose = true
    else
      opts.pattern[#opts.pattern + 1] = arg
    end
    i = i + 1
  end
end

local function render(scene, render_opts)
  local render = dort.render.make(scene, dort.std.clone(render_opts))
  local time_begin = dort.chrono.now()
  dort.render.render_sync(render)
  local time_end = dort.chrono.now()
  local out_image = dort.render.get_image(render, { hdr = true })
  local time_s = dort.chrono.difference_s(time_begin, time_end)
  return out_image, time_s
end

local tests = {}
local function add_test(t, test_def)
  assert(test_def.name, "name")
  assert(test_def.scene, "scene")
  assert(test_def.renders, "renders")
  tests[#tests + 1] = test_def
end

local function run_test(test_def)
  local ref_path = string.format("%s/ref/%s.hdr", dir, test_def.name)
  local out_dir_path = string.format("%s/out/%s", dir, test_def.name)

  local function matches_pattern()
    if not opts.pattern then return true end

    for _, pattern in ipairs(opts.pattern) do
      if not string.find(test_def.name, pattern) then return false end
    end
    return true
  end

  local function generate_ref()
    local ref_opts = test_def.ref_opts
    if not ref_opts then return end

    if opts.verbose then
      dort.std.printf("  generating ref %s...", ref_opts.renderer)
    else
      dort.std.printf("generating ref %s %s...", test_def.name, ref_opts.renderer)
    end
    io.stdout:flush()

    local out_image, ref_time_s = render(scene, ref_opts)
    dort.image.write_rgbe(ref_path, out_image)
    dort.std.printf(" %sdone (%g s)%s\n", COLORS.cyan, ref_time_s, COLORS.reset)
  end

  local function compare_with_ref(render_def, ref_image, out_image)
    local x_res, y_res = dort.image.get_res(ref_image)
    local _, variance = dort.image.bias_variance(ref_image, out_image,
      dort.geometry.recti(0, 0, x_res, y_res))
    local norm_sd = dort.math.sqrt(variance / (3 * x_res * y_res))

    local conv_error = dort.image.test_convergence(ref_image, out_image, {
      min_tile_size = render_def.min_tile_size or 32,
      variation = render_def.variation or 8,
      p_value = 0.01,
    })

    local status
    if conv_error then
      status = "err"
    elseif norm_sd > 1.5 then
      status = "warn"
    else
      status = "ok"
    end
    return status, conv_error
  end

  local count = 0
  local errs = 0
  local time_s = 0

  local function test_render(render_def, ref_image)
    assert(render_def.name, "name")
    assert(render_def.opts, "opts")
    if opts.render and render_def.name ~= opts.render then return end
    if opts.verbose then dort.std.printf("  %s...", render_def.name) end
    io.stdout:flush()

    local out_image, test_time_s = render(test_def.scene, render_def.opts)
    dort.image.write_rgbe(string.format("%s/%s.hdr",
      out_dir_path, render_def.name), out_image)

    if opts.compare_with_ref then
      local status, conv_error = compare_with_ref(render_def, ref_image, out_image)
      if status == "err" then errs = errs + 1 end

      local color = ({
        ok = COLORS.green,
        warn = COLORS.yellow,
        err = COLORS.red,
      })[status]

      if opts.verbose then
        dort.std.printf(" %s%s (%g s)%s\n", color, status, test_time_s, COLORS.reset)
        if conv_error then dort.std.printf("    %s\n", conv_error) end
      elseif status ~= "ok" then
        dort.std.printf("%stest %s %s: %s%s\n", color, test_def.name,
          render_def.name, status, COLORS.reset)
        if conv_error then dort.std.printf("  %s\n", conv_error) end
      end
    elseif opts.verbose then
      dort.std.printf(" %sdone (%g s)%s\n", COLORS.cyan, test_time_s, COLORS.reset)
    end

    count = count + 1
    time_s = time_s + test_time_s
  end

  if not matches_pattern() then
    return 0, 0, 0
  end
  if opts.verbose then
    dort.std.printf("test %s\n", test_def.name)
  end
  if opts.generate_ref then
    generate_ref()
    return 0, 0, 0
  end

  -- TODO: nasty hack
  os.execute(string.format("mkdir -p %q", out_dir_path))
  local ref_image
  if opts.compare_with_ref then
    ref_image = dort.image.read(ref_path, { hdr = true })
    dort.image.write_rgbe(out_dir_path .. "/ref.hdr", ref_image)
  end

  for _, render_def in ipairs(test_def.renders) do
    test_render(render_def, ref_image)
  end
  return count, errs, time_s
end

local function run_tests()
  local total_begin = dort.chrono.now()
  local count = 0
  local errs = 0
  local time_s = 0
  for _, test_def in ipairs(tests) do
    local test_count, test_err, test_s = run_test(test_def)
    count = count + test_count
    errs = errs + test_err
    time_s = time_s + test_s
  end

  local total_time_s = dort.chrono.difference_s(total_begin, dort.chrono.now())
  if errs == 0 then
    dort.std.printf("%s%d tests passed (%g s, %g s)%s\n",
      COLORS.green, count, time_s, total_time_s, COLORS.reset)
  else
    dort.std.printf("%s%d/%d tests failed (%g s, %g s)%s\n",
      COLORS.red, errs, count, time_s, total_time_s, COLORS.reset)
  end
end

local t = {
  test = add_test,
}
local function load_tests(file)
  (dofile(dir .. "/" .. file))(t)
end

parse_argv(dort.env.get_argv())
load_tests("test_simple.lua")
load_tests("test_box.lua")
load_tests("test_bsdf.lua")
run_tests()
