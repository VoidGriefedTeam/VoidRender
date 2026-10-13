# .map guide (MAP2)

The text format you write by hand. `mapc level.map` compiles it to a binary `level.mapb` (version 2).
For the binary layout see `include/mapfmt/mapformat.hpp`. For lighting and rendering details see the VoidRender cheat sheet.

Angles are DEGREES. Orientation = Ry(yaw) * Rx(pitch) * Rz(roll). World is Y-up, camera yaw 0 looks down -Z.

---------------------------------------------------------------------------------------------------

## 1. Quick example

    Map = "town"
    Objects = 1

    Ambient = 0.12, 0.12, 0.16

    Light {
       Type = point
       Position = 0, 8, 0
       Color = 1, 0.85, 0.6
       Intensity = 1.2
       Range = 40
       Orbit { Center = 0,0,0  Radius = 6  Height = 8  Speed = 45 }
    }
    Light { Type = directional  Direction = -0.3, -1, -0.2  Intensity = 0.4 }

    Object house:
       x = {-1, 1, 1, -1}
       y = {-1, -1, 1, 1}
       z = {0, 0, 0, 0}
       yaw = 30
       pivot = 0, 0, 0
       faces = {{0,1,2},{0,2,3}}
       colors = {{1,0,0},{0,1,0},{0,0,1},{1,1,0}}

    Camera cam:
       x = 0
       y = 5
       z = 10
       fov = 90
       yaw = 0
       pitch = -10

---------------------------------------------------------------------------------------------------

## 2. File structure

    Map = "name"            required, must be first
    Objects = N             required, must be second; must equal the number of Object blocks
    Ambient = ...           optional, once
    Light { ... }           optional, any number
    Object name: ...        any number (N of them)
    Camera name: ...        required, exactly one

After the two header lines, `Ambient`, `Light`, `Object` and `Camera` can come in any order.

## 3. Syntax rules

- **Comments:** `#` or `//` to end of line.
- **Whitespace:** free-form. Newlines don't matter, so `Orbit { Radius = 6  Speed = 45 }` on one line is fine.
- **Numbers:** `5`, `-0.25`, `+3`, `1e3`. No hex.
- **Strings:** `"in double quotes"`, no escapes, no line breaks. Only used for the map name and `Type`.
- **Keys:** case-insensitive, and `_` is ignored. `FollowCamera`, `follow_camera` and `FOLLOWCAMERA` are the same.
  The block words `Object`, `Camera`, `Light`, `Ambient` are case-insensitive too.
  The two header words `Map` and `Objects` must be written exactly like that.
- **`=`:** optional between a key and its value (`rotation {}` is fine).
- **Vectors (3 numbers):** write `a, b, c` or `{a, b, c}`. Both mean the same.
- **Lists:** `{1, 2, 3}`. Nested: `{{0,1,2},{0,2,3}}`. Empty: `{}`.
- **Booleans:** `true` / `false` (also `yes`/`no`, `on`/`off`, `1`/`0`).

---------------------------------------------------------------------------------------------------

## 4. Ambient

    Ambient = 0.12, 0.12, 0.16

Flat light added everywhere, RGB 0..1. Default `0.15, 0.15, 0.15`. Only used when there is at least one Light
(no lights = lighting off, colours drawn as stored).

## 5. Light

    Light { ... }               # an optional name after the word Light is allowed and ignored

| key | value | default | meaning |
|---|---|---|---|
| `Type` | `point` or `directional` | `point` | kind of light |
| `Position` | vec3 | `0,0,0` | Point: world position (with `FollowCamera`: offset in camera space) |
| `Direction` | vec3 | `0,-1,0` | Directional: the way the light TRAVELS, `-0.3,-1,-0.2` = from above. Must not be `0,0,0`. |
| `Color` | vec3 (RGB) | `1,1,1` | `Colour` also works |
| `Intensity` | number | `1` | multiplies Color; values above 1 are fine |
| `Range` | number >= 0 | `0` | Point: fades to 0 at this distance. `0` = no falloff |
| `FollowCamera` | boolean | `false` | Point: `Position` is an offset in camera space (x right, y up, -z forward). `0,0,0` = headlight |
| `Orbit { ... }` | block | none | Point: circles in the XZ plane (see below) |

### Orbit

    Orbit { Center = 0,0,0  Radius = 6  Height = 8  Speed = 45  Phase = 0 }

| key | default | meaning |
|---|---|---|
| `Center` | `0,0,0` | middle of the circle |
| `Radius` | `0` | size of the circle. `0` = no orbit, light stays at `Position`. Must be >= 0. |
| `Height` | `0` | added to Center.y |
| `Speed` | `0` | degrees per second, negative = other way |
| `Phase` | `0` | start angle in degrees |

    angle = speed * time + phase
    pos   = center + ( cos(angle)*radius,  height,  sin(angle)*radius )

`Range` is how far the light shines. `Orbit Radius` is how big its circle is. They are unrelated.
Orbit only applies to point lights; a directional light ignores it.

---------------------------------------------------------------------------------------------------

## 6. Object

    Object name:
       x = {...}   y = {...}   z = {...}
       faces = {{...}, ...}
       colors = {{...}, ...}
       ...

| key | value | default | meaning |
|---|---|---|---|
| `x`, `y`, `z` | list of numbers | empty | vertex coordinates. All three must have the same length. |
| `faces` | list of index lists | empty | each face is a list of vertex indices, `{0,1,2}`. Every index must be in `0..vertex_count-1`. Wind faces counter-clockwise seen from OUTSIDE. |
| `colors` (or `color`) | list of float lists | empty | per-vertex colours, floats 0..1 (first 3 = RGB, extra components just pass through) |
| `rotation` | list of numbers | empty | legacy rotation list, stored as-is |
| `yaw`, `pitch`, `roll` | number (degrees) | `0` | object rotation, about the pivot |
| `pivot` | vec3 | centroid | sets a custom pivot (`custom_pivot = true`). Leave it out to rotate about the centroid of the vertices. |

The object block ends where the next `Object`, `Camera`, `Light` or `Ambient` begins.
`mapc` does not check how many colour rows you give. VoidRender expects one per vertex.

`visible` and `emissive` are not in the file. Set them in code at runtime.

---------------------------------------------------------------------------------------------------

## 7. Camera

    Camera name:
       x = 0   y = 5   z = 10
       fov = 90
       yaw = 0   pitch = -10   roll = 0

| key | default | meaning |
|---|---|---|
| `x`, `y`, `z` | `0` | position (single numbers) |
| `fov` | `90` | degrees, horizontal |
| `yaw` (or `dir`) | `0` | turn left/right, + = left |
| `pitch` | `0` | + = look up |
| `roll` | `0` | + = head tilts left |
| `rotation` | empty | legacy rotation list, stored as-is |

Exactly one Camera is required.

---------------------------------------------------------------------------------------------------

## 8. Errors mapc reports

- `Objects = N` doesn't match the number of Object blocks
- missing Camera, or more than one
- `x`, `y`, `z` of an object have different lengths
- a face index is out of range
- a vector doesn't have exactly 3 numbers
- unknown key (object, camera, light or orbit field)
- unknown light type (only `point` and `directional`)
- `Range` or `Orbit Radius` below 0
- a directional light with `Direction = 0,0,0`
- an unclosed `{`, an unterminated string, or an invalid number

Messages include the line number where it helps.

---------------------------------------------------------------------------------------------------

## 9. Compiling

    mapc level.map                 # writes level.mapb next to it
    mapc level.map out/level.mapb  # explicit output path

In CMake: `mapfmt_add_map(my_maps INPUTS levels/town.map OUTPUT_DIR "${CMAKE_BINARY_DIR}/maps")`.

Loading in C++: `auto map = mapfmt::load_map("level.mapb");` fills `Map::lights`, `Map::ambient`,
`camera.dir/pitch/roll` and the object `yaw/pitch/roll/custom_pivot/pivot`. Version 1 files still load
(no lights, ambient 0.15, all angles 0).
