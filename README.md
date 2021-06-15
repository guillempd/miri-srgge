# README

## Features Implemented

### Session 1

* First-person style camera that allows to traverse the scene using WASD buttons + mouse. Buttons Q and E can also be used to control the elevation of the camera.
* Compute and display frame rate using Dear ImGui library.

### Session 2

* Museums can be imported as the combination of three files: *.tm, *.m and *.v (more on them in the Loading a Museum section)

### Session 3

* Octree based vertex clustering simplification method.
* Provided as a separated executable (MeshSimplifier).

### Session 4

* QEM method for computing the representative of vertex clustering using Eigen library for solving the linear system.

### Session 5

* Time critical rendering implementation.
* TPS is user controlled by using the provided slider.

### Session 6

* Ray based visibility precomputation, using the Bresenham supercover approach.
* Provided as a separated executable (VisibilityPrecomputation)

## Compilation

The CMake provided generates the three executables: BaseCode, MeshSimplifier and VisibilityPrecomputation.

## Loading a Museum

A museum is composed of the following text files:

* A list of models (*.m)
* A floor plan (*.tm)
* A visibility description of the scene (*.v)

The first two are user generated and the third has to be generated using the VisibilityPrecomputation program.

All of them should have matching names.

One complete example of each of them is provides under the directory scenes.

By default the BaseCode reads these three, a command line argument can be passed to load other museum files. For example: ./BaseCode my_museum
### Models File Structure (*.m)
The models file indicates which models should be loaded and which character is associated to them, so that it can be instantiated in the floor plan.
The first line contains the amount of models to load.

### Floor Plan File Structure (*.tm)
The floor plan contains a grid of characters representing the floor plan. A '.' represents an empty space, an 'x' represents a wall and other characters represent the corresponding model associated in the models file.
The first line contains the width and the height of the floor plan.

### Visibility File Structure (*.v)
The visibility file contains a line for each cell in the floor plan.
For each line, the first two numbers indicate the cell and the following pairs of numbers indicate the cells that are visible from that cell and contain some statue.

## Visibility Precomputation

The visibility precomputation program expect the following input:

1) Floor plan of the museum we wish to compute its visibility (*.tm file)
2) Number of rays to sample through the scene ( if not provided it will default to 1M, which is enough in all the cases I have tested).

## Mesh Simplifier

The mesh simplifier program expects the following input:

1) Path of the model to simplify
2) Method for computing the representative: either "mean" or "qem" (without quotes)
3) Max depth of the octree
4) Amount of levels of detail to compute

At any point, the user might decide to not provide more parameters which implies that all of the ones that have not been specified will take default values.

Examples:

./MeshSimplifier models/lucy.ply mean 6 1

./MeshSimplifier models/dragon.ply qem 8

./MeshSimplifier models/torus.ply

The output of running this program will be several files with the names i.ply containing the i-th level of detail for the specified input.

Levels of detail are sorted in increasing order, meaning that higher number implies more complex models.