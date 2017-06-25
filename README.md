# dort

`dort` is a physically based renderer. It is still under heavy development, so
do not expect it to be usable in general (and correct in particular).

## Building

`dort` carries most of its dependencies in the repository (in `extern/`), the
only library that is necessary by default is **`libz`**.

To build `dort`, one also needs a C++ compiler (**clang** and **gcc** should
work), **objcopy** (part of GNU binutils), and **tup**. `tup` can be downloaded
from <http://gittup.org/tup/>, either as a build-it-yourself repository or as a
Ubuntu package.

The build can be configured using file `tup.config` (see the template
`tup.config.default`). By default, only the development version of the program
is built; other versions can be enabled using `CONFIG_FAST=y` (fast, optimized
build) or `CONFIG_SLOW=y` (slow, debug build). The different versions of the
binary will be built in different directories:

- development version -- `build/d/dort`
- fast version -- `build/f/dort`
- debug version -- `build/g/dort`

To build all enabled versions, run `tup`. Use `tup build/{d,f,g}/dort` to
compile only a single version.

If enabled, `dort` can also link to Gtk to support a simplistic GUI, but this is
somewhat deprecated and will probably be removed in the future. Otherwise, the
only runtime dependency is to the default `libc`, other libraries should be
linked in statically.

## Usage

The produced `dort` binary is a Lua interpreter that exposes the rendering API
in Lua. To run a Lua program, invoke `dort <program.lua>`.

To obtain the documentation of the Lua API, install
[ldoc](https://github.com/stevedonovan/LDoc) and run `ldoc .` in the repository.
The generated documentation will be stored in `.ldoc/`. We do not publish the
documentation on a public website because `dort` is not quite ready for this
yet.

## Samples

Example Lua scripts that render images are stored in the `samples/` directory.
Do not be deceived into opening `examples/`! It contains various files that have
been used during development and that were convenient to store in git.

Some historical images documenting the progress of development can be found in
`images/`.
