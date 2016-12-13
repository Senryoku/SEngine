# SEngine

OpenGL Playground.
It's mainly a Deferred rendering engine.

The main optimisation is the use of a compute shader to divise the computation of the lights contributions by tiles. A workgroup is assigned to each tile and determines which lights contributes to this particular tile. This way we can have hundreds of dynamic point lights without greatly affecting the performances. 

Quick video:
https://youtu.be/NRjylzEVU9w

# TODO
## Major
 * Clean the ECS API (A LOT!)
 * Convert Light(s) to components (using Transformation for positionning and direction)
 * A true Scene manager... (and an octree)
 * Develop a scene file format and use it.
 * Write a editor for this format.
 * Decouple the renderer from the application ? (related to the previous point)

## Minor
 * Split meshes with multiple materials into differents objects
 * ...

# Dependencies

 * [GLFW](http://www.glfw.org/)
 * [GL3W](https://github.com/skaslev/gl3w)
 * [SenOGL](https://github.com/Senryoku/SenOGL)
 * [GLM](http://glm.g-truc.net/0.9.7/index.html)
 * [tinyobjloader](https://github.com/syoyo/tinyobjloader) (included)
 * [dear imgui](https://github.com/ocornut/imgui) (included)
 * [nlohmann's json for modern C++](https://github.com/nlohmann/json) (included)
