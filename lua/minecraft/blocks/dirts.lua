local _ENV = require "minecraft/blocks/_env"

b.define("grass", 2, {
  bottom = m.make_matte {
    color = rgb(152, 188, 91) / 256,
    sigma = 0.2
  },
  side = m.make_matte {
    color = rgb(134, 96, 67) / 256,
  },
})

b.define("dirt", 3, m.make_matte {
  color = rgb(134, 96, 67) / 256,
})

b.define("sand", 12, m.make_matte {
  color = rgb(219, 211, 160) / 256,
})

b.define("gravel", 13, m.make_matte {
  color = rgb(127, 124, 123) / 256,
})
