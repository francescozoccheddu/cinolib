# cinolib
A generic programming header only C++ library for processing polygonal and polyhedral meshes. CinoLib supports surface meshes made of triangles, quads or general polygons as well as volumetric meshes made of tetrahedra, hexahedra or general polyhedra. A distinctive feature of the library is that all supported meshes inherit from a unique base class that implements their common traits, permitting to deploy algorithms that operate on _abstract_ meshes that may be any of the above. This allows to implement algorithms just once and run them on any possible mesh, thus avoiding code duplication and reducing the debugging effort.

<p align="center"><img src="cinolib_rep_image.png" width="500"></p>

## Usage
CinoLib is header only. It does not need to be installed, all you have to do is to clone the repo with
```
git clone https://github.com/mlivesu/cinolib.git
```
and include in your C++ application the header files you need. While for very small projects this could already be done by directly calling the compiler via command line with the `-I` option, it is strongly suggested to rely on a building system that can also handle optional external dependencies, compilation flags and symbols. CinoLib supports CMake, which is the standard the facto.

## Build a sample project (with CMake)
Here is an example of a toy program that reads a triangle mesh and displays it on a window
```
#include <cinolib/meshes/drawable_trimesh.h>
#include <cinolib/gl/glcanvas.h>

int main(void)
{
    using namespace cinolib;
    DrawableTrimesh<> m("bunny.obj");
    GLcanvas gui;
    gui.push(&m);
    return gui.launch();
}
```
and this is the `CMakeLists.txt` that can be used to compile it
```
project(test)
add_executable(${PROJECT_NAME} main.cpp)
set(CINOLIB_USES_OPENGL_GLFW_IMGUI ON)
find_package(cinolib REQUIRED)
target_link_libraries(${PROJECT_NAME} cinolib)
```
Compiling should be as easy as opening a terminal in the folder containing the two files above and type
```
mkdir build
cd build
cd .. -DCMAKE_BUILD_TYPE=Release -DCinoLib_DIR=<path-to-cinolib>
```
Note that for the rendering part CinoLib uses GLFW, which will be automatically installed and linked by the script `cinolib-config.cmake`, contained in the main directory. 
The same script can automatically download and install any other external dependency, meaning that if you want to access a functionality that depends from an external library all you have to do is setting to `ON` a cmake variable that looks like `CINOLIB_USES_XXX`. 
Valid options are
* `CINOLIB_USES_OPENGL_GLFW_IMGUI`: optional, used for rendering
* `CINOLIB_USES_OPENGL_TRIANGLE`: optional, used for polygon triangulation
* `CINOLIB_USES_OPENGL_TETGEN`: optional, used for tetrahedralization
* `CINOLIB_USES_EXACT_PREDICATES`: optional, used for robust geometry processing (e.g. exact detection of intersections) 
* `CINOLIB_USES_GRAPH_CUT`: optional, used for graph clustering
* `CINOLIB_USES_BOOST`: optional, used for 2D polygon operations (thickening, clipping, booleans...)
* `CINOLIB_USES_VTK`: optional, used just to support VTK file formats


## Other examples
A tutorial with detailed info on how to use the library is under developement. In the meanwhile, you can explore the [**examples**](https://github.com/mlivesu/cinolib/tree/master/examples#examples)  folder, which contains a constantly growing number of sample projects that showcase the core features of the library, and will be the backbone of the forthcoming tutorial.

## Contributors
Marco Livesu is the creator and lead developer of the library. Over the years various friends and colleagues have helped me to improve the codebase, either submitting code or helping me to spot and fix bugs. A big thanks goes to: Claudio Mancinelli (University of Genoa), Daniela Cabiddu (CNR IMATI), Chrystiano Araújo (UBC), Thomas Alderighi (CNR ISTI), Fabrizio Corda (University of Cagliari), Gianmarco Cherchi (University of Cagliari) and Tommaso Sorgente (CNR IMATI)

## Citing us
If you use CinoLib in your academic projects, please consider citing the library using the following 
BibTeX entry:

```bibtex
@article{cinolib,
  title   = {cinolib: a generic programming header only C++ library for processing polygonal and polyhedral meshes},
  author  = {Livesu, Marco},
  journal = {Transactions on Computational Science XXXIV},
  series  = {Lecture Notes in Computer Science},
  editor  = {Springer},
  note    = {https://github.com/mlivesu/cinolib/},
  year    = {2019},
  doi     = {10.1007/978-3-662-59958-7_4}}
```

## Acknowldegment
The software collected in CinoLib spans across a large period of time, starting from the beginning of my PhD to today. Since 2015, this work has been partly supported by various research projects, such as
* [CAxMan](https://cordis.europa.eu/project/id/680448)
* [CHANGE](https://cordis.europa.eu/project/rcn/204834/en)

