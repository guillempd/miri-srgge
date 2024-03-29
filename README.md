# MIRI-SRGGE Project (Scalable Rendering for Graphics and Game Engines)

## Project description
This project consists on a 3D renderer that allows to view a museum with several statues (3D models) of varying complexity.
With the goal of achieving a high performance, several optimization strategies such as LODs, time critical rendering and visibility precomputation are used. See [here](#key-optimizationfeatures-implemented) for more details.

## Demo Video

https://user-images.githubusercontent.com/15831136/138288925-8e7f39f7-91f7-4983-811c-9e9d565d0251.mp4

## Building the project

The project is provided as a CMake project. In order to build it (Linux) execute the following commands at the root directory (where the `CMakeLists.txt` file is located):

```
mkdir build
cd build
cmake ..
make
```

This series of commands will generate three executables:

- `BaseCode`
- `MeshSimplifier`
- `VisibilityPrecomputation`

## Loading a Museum

A museum description is defined by the following text files:

* A list of models (`*.m`)
* A floor plan (`*.tm`)
* A visibility description of the museum (`*.v`)

The first two are user generated and the third has to be generated using the `VisibilityPrecomputation` program as explained [here](#generating-the-v-file).

All of them should have matching names.

One complete example of each of them is provided under the `scenes` directory.

By default, `BaseCode` reads the three already provided, however, a command line argument can be passed to load other museum files. For example: running `./BaseCode my_museum` will load `my_museum.m`, `my_museum.tm` and `my_museum.v`. 
### Models File Structure (`*.m`)
The models file indicates which models should be loaded and which character is associated to them, so that they can be instantiated in the floor plan.
The first line contains a number indicating the amount of models to load.

### Floor Plan File Structure (`*.tm`)
The floor plan contains a grid of characters representing the floor plan. A `.` represents an empty space and an `x` represents a wall. Other characters represent the corresponding model associated in the models file.
The first line contains the width and the height of the floor plan.

### Visibility File Structure (`*.v`)
The visibility file contains a line for each cell in the floor plan.
For each line, the first two numbers specify the cell coordinates and the following pairs of numbers indicate the coordinates of the cells that are visible from that cell and contain some statue.


## Generating the `*.v` file

The `*.v` file is generated by the `VisibilityPrecomputation` command line program.

This program expects the following input:

1) Floor plan of the museum we wish to compute its visibility (`*.tm` file)
2) Number of rays to sample through the scene (if not provided it will default to 1M, which is enough in all the cases I have tested)

Example:

`./VisibilityPrecomputation my_museum.tm`

The output of this program will be the `my_museum.v` file.

## Generating the LODs

The LODs required to run the program are generated by the `MeshSimplifier` command line program and stored in the `/models` folder in their corresponding directory.

This program expects the following input:

1) Path of the model to simplify
2) Method for computing the representative: either `mean` or `qem`
3) Max depth of the octree
4) Amount of levels of detail to compute

At any point, the user might decide to not provide more parameters which implies that all of the ones that have not been specified will take default values.

Examples:

`./MeshSimplifier models/lucy.ply mean 6 1`

`./MeshSimplifier models/dragon.ply qem 8`

`./MeshSimplifier models/torus.ply`

The output of running this program will be several files with the names `i.ply` containing the i-th level of detail for the specified input.

Levels of detail are sorted in increasing order i.e. higher number implies more complex models.

## Navigating Through the Museum

Navigation through the museum is done using a First Person Shooter style camera: use WASD keys to move around and mouse to look around. Q and E keys are also enabled to change the elevation of the camera. This is useful to see how objects that are not supposed to be visible (since the observer is assumed to be at ground level) are not rendered thanks to the visibility precomputation.

## Using the Interface
By default the mouse input is captured and so the interface **can't** be used. To stop capturing the mouse and be able to use the interface (as well as resizing the screen) use the key `i`.

The debug colors allow to easily identify what is the current level of detail of an statue: in increasing order of level of detail the colors are red, orange, yellow and green.

The interface also has a slider that allows to modify the Triangle Per Second (TPS) parameter of the time critical rendering algorithm. With the debug colors enabled it is easy to see how increasing TPS the LODs of the statues also increase, specially those of nearby statues.


## Key Optimization/Features Implemented

### LOD generation by using a vertex clustering approach [[1]](#1)

In order to be able to generate all the different required LODs at the same time, an octree data structure containing the vertices of the model is built. 

With that, obtaining the representative of a cluster just means processing all the vertices in a node of the octree.

Also, generating a model with lesser level of detail can be done easily by propagating the information from the leaves to their parent.

### Representative computation: centroid or Quadric Error Method (QEM) [[2]](#2)
The representatives of each of the aforementioned clusters can be done by computing the centroid or by a more sophisticated approach using QEM.

### Time critical rendering implementation [[3]](#3)
The LOD selection of each statue is performed by solving a small optimization problem: at each frame, the LODs are selected so that the visual quality of the rendered image is maximized but respecting a maximum number of Triangles Per Second (TPS) so that the frame rate remains acceptable at all time.

This problem is similar to solving a multiple-choice knapsack problem and it is solved by using a greedy algorithm.

### Visibility precomputation of the scene

The potentially visible set (PVS) of each cell of the museum is precomputed.

At runtime, only the statues that are contained in the PVS of the current cell are considered for the time critical rendering algorithm.

The visibility is precomputed using an approximate method based on sampling random rays throught the museum floor plan.

## References

<a id="1">[1]</a>
Rossignac, J., & Borrel, P. (1993). Multi-resolution 3D approximations for rendering complex scenes. In Modeling in Computer Graphics (pp. 455–465). Springer Berlin Heidelberg. https://doi.org/10.1007/978-3-642-78114-8_29

<a id="2">[2]</a>
Garland, M., & Heckbert, P. S. (1997). Surface simplification using quadric error metrics. Proceedings of the 24th Annual Conference on Computer Graphics and Interactive Techniques - SIGGRAPH ’97, 209–216. https://doi.org/10.1145/258734.258849

<a id="3">[3]</a>
Funkhouser, T. A., & Séquin, C. H. (1993). Adaptive display algorithm for interactive frame rates during visualization of complex virtual environments. Proceedings of the 20th Annual Conference on Computer Graphics and Interactive Techniques - SIGGRAPH ’93, 247–254. https://doi.org/10.1145/166117.166149
