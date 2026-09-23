# ChessWithMeMate

A 3D chess board built in C++ with legacy OpenGL and GLUT. Pieces are picked with the mouse using ray casting, move with a short hopping animation, and captured pieces tip over and fade out.

Built as my final project for **CS 450/550 (Computer Graphics), Oregon State University, Fall 2023**.

## Features

- **Ray-cast picking.** A mouse click is unprojected through the stored projection and view matrices into a world-space ray, which is tested against per-piece and per-tile axis-aligned bounding boxes.
- **3D models.** Each piece type is loaded from a Wavefront `.obj` file in [`pieces/`](pieces/).
- **Move validation per piece type.** Pawn (single/double step, diagonal capture), knight, bishop, rook, queen, and king moves are checked, including blocked paths for sliding pieces.
- **Animation.** Moving pieces follow a sine-wave arc to their target square; captured pieces rotate over and fade out.
- **Lighting.** A switchable scene light with selectable color.
- **Debug view.** Draws the pick ray and bounding boxes and logs selection details to the console.

## Controls

| Input | Action |
| --- | --- |
| Left-click a piece, then a square or enemy piece | Select, then move or capture |
| Shift + left-drag | Rotate the camera |
| Scroll wheel | Zoom |
| Right-click | Menu (view, projection, axes, depth options, debug, reset, quit) |
| `l` | Toggle lighting |
| `w` `r` `g` `b` `y` | Light color: white, red, green, blue, yellow |
| `f` | Freeze/unfreeze animation |
| `q` / `Esc` | Quit |

## Building

### Windows (Visual Studio)

1. Open `Sample.sln` in Visual Studio 2022 (toolset v143, Win32).
2. Build and run the **Debug | Win32** configuration.

GLEW, freeglut, and GLM are vendored in the repo (`glew.h`, `freeglut*.h`, `glm/`, `*.lib`), so there is nothing else to install. Run the program from the project root: the `.obj` models are loaded from `pieces/` relative to the working directory, and `freeglut.dll` / `glew32.dll` are picked up from there. That's Visual Studio's default when debugging.

### Linux

A minimal `Makefile` from the course template is included:

```sh
make
./sample
```

This needs the OpenGL, GLU, and freeglut development packages (e.g. `libgl-dev libglu1-mesa-dev freeglut3-dev`) plus GLEW. The project was developed and tested on Windows, so the Linux build may need small adjustments.

## Project layout

```
sample.cpp          Main program: board, pieces, picking, move rules, rendering, input
loadobjfile.cpp     .obj model loader
setlight.cpp        Lighting helpers
setmaterial.cpp     Material helpers
osu*.cpp, keytime*  Course-provided utilities (shapes, keyframe timing)
bmptotexture.cpp    BMP texture loader (course-provided)
pieces/             Chess piece models
glm/                GLM math library (vendored)
```

## Known limitations

This was scoped as a graphics project, so the chess rules are intentionally incomplete:

- Turn order is not enforced; either side can move at any time.
- No check, checkmate, or stalemate detection.
- No castling, en passant, or pawn promotion.
- Textures were cut in favor of solid-color pieces and board.

## Acknowledgments

Built on the Oregon State CS 450/550 course sample framework. Uses [GLM](https://github.com/g-truc/glm), [GLEW](https://glew.sourceforge.net/), and [freeglut](https://freeglut.sourceforge.net/).
