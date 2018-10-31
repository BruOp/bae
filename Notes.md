# Notes

## Uniforms

Okay so the problem for the uniforms is basically the following:

- Some uniforms are the same for the scene
  - Light positions, directions, properties
  - Camera position, projection and view transforms
- Some uniforms are the same for every material type?
- Many uniforms are different for each and every mesh instance

  - Color, textures (?? PROBABLY ??)

- Each material type is defined as an instance, not a class
  - How do we tie a set of uniforms to an instance of material type?
  - How do we seperate uniforms that change per scene, per material type, and per material?
  - We have a few options:
    - A mapping between material types and the following types of "uniform sets"
      - per scene??
      - per material type
      - per material instance
- Who owns the uniforms required for a material type? The type? Or is the relation ship more indirect
  - Implicit means that we have the mapping, but there is not direct ownership between the material type to the set.
    - Allows us to create the two at different times, and to have the actual material type act as the key in the mapping. (We probably shouldn't do this, but we can use the name)
  - Explicit means we can use the material type as a unit.
    - Will require duplicate work, as both the material type and the material might 'own' their own sets of uniforms.
      - This duplicate work might be inevitable, and providing a clear interface may actually be the correct thing to do
      - The usage may be sufficiently different that it doesn't make sense to force this contract.
- For now, we'll go with an indirect relationship
- We'll have a set of "standard" uniforms that will be used in all of our shaders, like lights (avoid for now)

- Okay so for a specific program, we need only send "updates" to the uniforms through `setUniform`.

- If this is the case then we can definitely say that there are sets of uniforms, and each uniform set can be arranged by frequecy, ie:

  - per scene (??)
  - per frame (lights)
  - per material type (??)
  - per mesh instance

- The material type can define a set of uniforms that the mesh must define. All the per frame and per scene stuff is foggier.

  - The material types can probably provide that set as well, since the bgfx pipeline allows it. In vulkan, we'd have to do that there as well since AFAIK pipelines are responsible for this shit, and they are totally independent and immutable. There though you can define sub pipelines and stuff.

- Okay so then our render loop should look something like:

```c++
for material_types in material_types:
  set material_type properties as uniforms
  for geometry in geometries:
    set geometry stuff
      for meshes in meshes_with_material:
        set mesh instance uniforms
        draw()
```

So what's the problem here?

- Not all geometries should get drawn for material_types...
- Subsetting our meshes by material and geometry type is totally different from how we're currently doing it.

Alternatives? Don't worry about state changes, but worry about memory access

- We definitely need a construct that manages uniforms
  - Don't know that we need to set the uniforms again with every material type, but lets assume that we do for now. That would simplify our logic to maybe look more like:

```c++
for mesh_group in mesh_groups:
    set mesh_group.material_type
    for mesh in mesh_group.meshes:
        load_geometry() // ??
        set mesh instance uniforms
        draw()
```

The big quesiton mark here is still just how we perform the `load_geometry()` step without missing the cache. Or if this 'optimization' even matters. But for now, I just won't worry about it. If we allow copying onf the Geometry, and have a "GeometryManager" handle any destruction required, then we wouldn't have to worry about cache misses -- the mesh stores the handle, we don't have to load anything new into memory if we store the handle itself.

This seems like a really important point -- allowing copying and passing around of handles helps us potentially **avoid** cache misses!

OKAY So that's the final solution we'll go with -- have the mesh store the geometry handle.

## General Design notes

BGFX does NOT follow RAII. So we have a few options here:

1. Also don't follow RAII, instead implement `init` and `destroy` methods ourselves
2. Follow RAII loosely, still allow `init` and `destroy` methods but ensure that our destructors always clean up after themselves
3. Follow RAII completely. Wrap BGFX wherever we need to in order to allow this.

The pros and cons of each technique are outlined below:

### Use RAII Completely

Pros:

- Potentially safest. If we're strict about copying, destruction and initialization then we don't have to worry about dangling handlers and the like
- Wrappers can potentially be moved off bgfx in the future if we end up doing that

Cons:

- Will need to do a lot of wrapping to make sure we're covering the whole API that we're using.
- Sometimes very difficult to managment movement, copying without reducing expressiveness
- Not always easy to ensure that we're destroying an actual handle rather than the default
- Makes it hard to work with `null` objects, handles that haven't actually been initialized yet and the like. Which again makes things potentially a bit hard to work with.
- Makes it hard to share GPU resources properly
  - An example is a geometry (vertex + index buffer) which could potentially be rendered using many different programs. Similarly a program could render many different geometries. It's not clear how or if ownership should be enforced at that level.
    - AND YET we still need some sense of ownership, because ultimately things do need to be cleaned up at SOME point.

### Use RAII only when it obviously makes sense

### No RAII

Pros:

- Most flexibility in terms of what order we want to destroy things in

Cons:

- With flexibility comes risk of missing things
- Also makes ownership more difficult to keep track of
- Not part of the core guidelines.

I think the solution is to:

- Use RAII completely BUT always make sure to offer a default initializer and a minimally working state. Ideally, all our handles can be initialized to some known "invalid" value.
