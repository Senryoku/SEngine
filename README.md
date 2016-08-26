# SEngine

OpenGL Playground.
It's mainly a Deferred rendering engine.

The main optimisation is the use of a compute shader to divise the computation of the lights contributions by tiles. A workgroup is assigned to each tile and determines which lights contributes to this particular tile. This way we can have hundreds of dynamic point lights without greatly affecting the performances. 

Quick video:
https://youtu.be/NRjylzEVU9w

# TODO
 * Split meshes with multiple materials into differents objects
 * Write true object model (ECS ?) and get rid of MeshInstance (or make it a component)
 * A true Scene manager... (and an octree)

# Dependencies

 * [GLFW](http://www.glfw.org/)
 * [GL3W](https://github.com/skaslev/gl3w)
 * [SenOGL](https://github.com/Senryoku/SenOGL)
 * [GLM](http://glm.g-truc.net/0.9.7/index.html)
 * [tinyobjloader](https://github.com/syoyo/tinyobjloader) (included)
 * [dear imgui](https://github.com/ocornut/imgui) (included)
