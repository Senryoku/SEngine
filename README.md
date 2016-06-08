# SEngine

OpenGL Playground.
It's mainly a Deferred rendering engine.

The main optimisation is the use of a compute shader to divise the computation of the lights contributions by tiles. A workgroup is assigned to each tile and determines which lights contributes to this particular tile. This way we can have hundreds of dynamic point lights without greatly affecting the performances. 

Quick video:
https://youtu.be/NRjylzEVU9w


[Font SDF are generated using SDFont (https://github.com/VDrift/sdfont)]