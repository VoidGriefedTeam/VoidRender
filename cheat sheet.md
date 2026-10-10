# mapfmt `.map` syntax cheat sheet

Text `.map` → `mapc` → binary `.mapb` → `mapfmt::load_map("x.mapb")`.

## Skeleton

```
# comment   (// comment also works)

Map = "name"
Objects = N                    # must equal the number of Object blocks

Object <name>:
   x = {…}
   y = {…}
   z = {…}
   rotation = {…}
   faces = {{0,1,2},{0,2,3}}
   colors = {{r,g,b},{r,g,b}}

Camera <name>:                 # exactly one, required
   x = 0
   y = 0
   z = 0
   fov = 90
   rotation = {}
```

## Fields

| Block  | Field      | Type                        | Notes                                      |
|--------|------------|-----------------------------|--------------------------------------------|
| Map    | `Map`      | `"string"`                  | first line                                 |
| Map    | `Objects`  | integer                     | must match the number of Object blocks    |
| Object | `x` `y` `z`| `{n, n, …}`                 | one entry per vertex; all three same length|
| Object | `rotation` | `{n, n, …}`                 | may be `{}`                                |
| Object | `faces`    | `{{i,i,i},…}`               | vertex indices, each 0..vertexCount-1      |
| Object | `colors`   | `{{r,g,b},…}`               | `color` is accepted as an alias            |
| Camera | `x` `y` `z`| single number               | not a list                                 |
| Camera | `fov`      | single number               | defaults to 90 if omitted                  |
| Camera | `rotation` | `{n, n, …}`                 | may be `{}`                                |

## Rules

- Order: `Map`, then `Objects`, then Object/Camera blocks in any order.
- Fields inside a block can be in any order. Omitted fields are empty.
- `=` is optional (`rotation {}` works).
- Whitespace and newlines are insignificant. Items in `{}` need commas; a trailing comma is OK.
- Names are bare identifiers (letters, digits, `_`), followed by `:`.
- Keywords are case-sensitive.
- A field can't be named `Object` or `Camera`.
- Numbers: `-1`, `+2`, `.5`, `45.5`, `1e3`.

## Compile errors

- `Objects = N` doesn't match the number of Object blocks
- missing `Camera`, or more than one
- x, y, z lengths differ
- face index out of range
- unknown field, unterminated string, missing `:` / `,` / `}`

Not checked: `colors` count vs `faces` count.

## Example

```
Map = "example map"
Objects = 1

Object quad:
   x = {-1, 1, 1, -1}
   y = {-1, -1, 1, 1}
   z = {0, 0, 0, 0}
   rotation = {0, 45.5, 0}
   faces = {{0,1,2},{0,2,3}}
   colors = {{1,0,0},{0,1,0}}

Camera cam:
   x = -85
   y = 5
   z = 2
   fov = 120
   rotation = {}
```

## Commands

```
mapc input.map [output.mapb]               # default output: input.mapb
cmake --workflow --preset release          # configure + build + test + package
```

```cpp
#include <mapfmt/MapReader.hpp>
auto map = mapfmt::load_map("town.mapb");  // std::expected<Map, mapfmt::Error>
```

```cmake
find_package(mapfmt CONFIG REQUIRED)
target_link_libraries(game PRIVATE mapfmt::mapfmt)
mapfmt_add_map(game_maps INPUTS levels/town.map OUTPUT_DIR "${CMAKE_BINARY_DIR}/maps")
add_dependencies(game game_maps)
```
