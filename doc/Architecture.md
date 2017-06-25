# `dort`

The `dort` renderer is heavily influenced by [pbrt](http://www.pbrt.org/)

`dort` is designed as a tight core of classes that provide interfaces between
various parts of the renderer. The implementations of these classes are then
largely independent of each other and of other parts of the system. The renderer
itself if written in C++, but exports itself as a Lua library. The `dort` binary
serves as an interpreter of Lua programs that access this API.

## Core

The core interfaces that form the basis of rendering are:

- `Primitive` -- a generic representation of an object in the scene that can be
  intersected by rays with associated shading information (in the form of
  `Material`). This notion allows us to handle various specialized types of
  geometry (in particular, different flavors of triangles), general `Shape`
  geometry and acceleration structures uniformly, so that hierarchies of
  primitives can be easily composed. Thus, all the geometry information in the
  scene is represented as a single `Primitive` to the rest of the system.

- `Shape` -- represents a generic geometric shape such as a triangle, sphere or
  a disk. Can be coupled with a `Material` and placed in the scene as a
  `Primitive` (`ShapePrimitive`); however, for large triangle meshes a more
  efficient representation is available.

- `Material` -- computes a `Bsdf` for a particular intersection of a
  `Primitive`, uses `Texture`s for parameters that vary in space.

- `Bsdf` -- directly corresponds to the theoretical concept of a BSDF,
  completely describes the scattering of light at a surface point. It provides a
  unified abstraction of shading to the renderers.

- `Light` -- represents emittors in the scene, also provides a unified interface
  for the renderers.

- `Camera` -- represents sensors in the scene.

### Primitives

The base `Primitive` interface provides methods for ray intersection and
for bounding the geometric extent of the primitive:

- `bool intersect(Ray& ray, Intersection& out_isect) const` -- determine the
  nearest intersection of `ray` with the primitive. If an intersection is found,
  the details are returned in `out_isect`.
- `bool intersect_p(const Ray& ray) const` -- determine whether the ray
  intersects the primitive, but does not need to determine the details; this
  special case is used for shadow tests and may be more efficient.
- `Box bounds() const` -- compute a box such that no intersection can occur
  outside of the box; this is used by the acceleration structures.

`GeometricPrimitive` represents (usually) a leaf in the hierarchy of primitives,
i.e. a primitive that corresponds to an actual geometric surface. It adds
methods to obtain the shading information at an intersection:

- `const Material* get_material(const Intersection& isect) const`
- `const Light* get_area_light(const DiffGeom& frame_diff_geom) const`

`DiffGeom` represents the local (differential) geometry at a point, such as the
location, normal and partial derivatives of the local _uv_-parameterization. An
`Intersection` stores the `const GeometricPrimitive*` that was hit by the ray
and two `DiffGeom`s that correspond to the local and world coordinate frames.

Some of the implemented `Primitives` are:

- `FramePrimitive` -- contains a single `Primitive` transformed by an affine
  transform. This allows for a single primitive to be inserted into the scene
  multiple times, sharing the same memory representation.

- `BvhPrimitive` -- list of `Primitives` stored in a BVH.

- `ShapePrimitive` -- represents a `Shape` transformed by an affine transform,
  with an associated `Material` and optional area `Light`. This is the most
  generic geometric primitive, but is quite expensive in memory.

- `TriangleShapePrimitive` -- corresponds to a single triangle in a triangle
  mesh (with the mesh data stored in a `Mesh` structure on side) and a reference
  to a `Material`. This representation is much more efficient than a
  `ShapePrimitive` with a `TriangleShape`, because it does not store any
  transformation (the mesh is assumed to have been correctly transformed
  before).

- `MeshTrianglePrimitive` -- is similar to `TriangleShapePrimitive`, but shares
  the `Material` reference for all triangles in the mesh, saving two
  pointer-sized fields in every primitive.

- `MeshBvhPrimitive` -- stores a list of triangles from a triangle mesh in an
  efficient BVH structure. This is much more efficient than having a unique
  primitive per triangle (both in terms of memory and CPU use).

### Bsdfs, Lights and Cameras

`Bsdf`s, `Light`s and `Camera`s all represent multi-dimensional functions that
appear in the integrals of light transport, and provide a set of methods for
sampling from their distributions and for computing the probability densities.

We often work with directions represented as unit vectors. As a convention, _wi_
always points to light and _wo_ always points to camera.

- **`Bsdf`** -- represents the surface scattering function _f(wi, wo)_ at a
  fixed point. It is separated into several lobes (diffuse, glossy and delta)
  that can be sampled independently.

  - `eval_f()` -- compute _f(wi, wo)_ for given _wi_ and _wo_.
  - `sample_light_f()` -- given, _wo_, sample _wi_. Used for bouncing rays from
    the camera to the light.
  - `sample_camera_f()` -- given _wi_, sample _wo_. Used for bouncing rays from
    the light to the camera.
  - `light_f_pdf()` -- compute the pdf of sampling _wi_ given _wo_ from
    `sample_light_f()`).
  - `camera_f_pdf()` -- compute the pdf of sampling _wo_ given _wi_ from
    `sample_camera_f()`).

  Delta components in the BSDF are explicitly marked as delta lobes and are
  always ignored by `light_f_pdf()` and `camera_f_pdf()`. We chose to explicitly
  differentiate the light and camera probability densities to make the code of
  the renderers simpler; this allows us to side-step most of the pits and
  pitfalls associated with "adjoint BSDFs".

- **`Light`** -- is an emittor. We represent emittors as functions _Le(x, wo)_
  or _Li(x, wi)_ that represent the radiance emitted from point _x_ in direction
  _wo_, respectively the light that is incident at point _x_ from direction
  _wi_.

  - `sample_ray_radiance()` -- samples _x_ and _wo_ (a ray) from the
    distribution of _Le(x, wo)_. This is used to shoot random rays into the
    scene for renderers that use light tracing.
  - `sample_pivot_radiance()` -- given a point _x_, samples a direction _wi_
    from the distribution of _Li(x, wi)_. This is used to efficiently estimate
    direct lighting at the point _x_ due to the light.
  - `sample_point()` -- sample a point _x_ on the light (such that _Le(x, wo)_
    is nonzero for some _wo_).
  - `eval_radiance()` -- evaluates _Li(x, wi)_ (where _wi_ is given as a point
    on the light).
  - `ray_radiance_pdf()` -- computes the pdf of sampling _Le(x, wo)_ from
    `sample_ray_radiance()`.
  -  `pivot_radiance_pdf()` -- computes the pdf of sampling _wi_ given _x_ from
     `sample_pivot_radiance()`.

  Delta lights may have delta distributions both in the positional and
  directional distributions (or both), which is communicated using flags
  associated with the light. There is also a special kind of distant lights that
  define only the distribution _Li(x, wi)_ (they are positioned infinitely away
  from the scene); they must be handled specially by the renderes.

- **`Camera`** -- represents a sensor in the scene. Cameras are similar to
  lights, they define functions _We(x, wi)_ and _Wi(x, wo)_ that model the
  distribution of emitted, respectively incident importance. However, they also
  associate a film position with each pair _(x, wi)_.

  - `sample_ray_importance()` -- samples _x_ and _wi_ (a ray) from the
    distribution of _We(x, wi)_ given a film position. This is used to shoot
    camera rays into the scene.
  - `sample_pivot_importance()` -- given a point _x_, samples a direction _wo_
    to the camera (given as a point on the camera) from _Wi(x, wo)_. This method
    is used to connect points in the scene to the camera.
  - `sample_point()` -- sample a point _x_ on the camera (such that _We(x, wi)_
    is nonzero for some _wi_).
  - `eval_importance()` -- compute _We(x, wi)_ for given _x_ and _wi_.
  - `ray_importance_pdf()` -- computes the pdf of sampling _x_ and _wi_ from
    `sample_ray_importance()`.
  - `pivot_importance_pdf()` -- computes the pdf of sampling _wi_ (given as a
    point on camera) from `sample_pivot_importance()`.

## Renderers

`dort` implements the following rendering algorithms:

- **path tracing** -- sample random walks from the camera and estimate direct
  lighting at the vertices.
- **light tracing** -- sample random walks from lights and connect the vertices
  to the camera.
- **bidirectional path tracing** -- sample a random walk both from light and
  from the camera, connect their vertices and combine the contributions using
  multiple importance sampling.
- **vertex connection and merging** -- sample random walks from the camera and
  from the lights, connect each camera walk with a single light walk and merge
  every camera vertex with all light vertices (photons) in a small radius; the
  contributions are again combined using multiple importance sampling.

The algorithms are abstracted away from the details of geometry and work mostly
with `Bsdf`s, `Light`s and `Camera`s, using only the generic interfaces.
However, there are various special cases (delta distributions, distant lights)
that must often be handled explicitly.
