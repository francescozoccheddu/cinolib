cmake_minimum_required(VERSION 3.2)

# https://cmake.org/cmake/help/git-stage/module/FetchContent.html
include(FetchContent)

option(CINOLIB_HEADER_ONLY "Use cinolib as header-only library" OFF)

#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# BASIC HEADER ONLY LIBRARY :::::::::::::::::::::::::::::::::::::::::::::
#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#interface-libraries
if (CINOLIB_HEADER_ONLY)
	set(CINOLIB_ACCESS INTERFACE)
	add_library(cinolib INTERFACE)
else()
	set(CINOLIB_ACCESS PUBLIC)
	file(GLOB_RECURSE CINOLIB_SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/include/cinolib/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/include/cinolib/*.h")
	add_library(cinolib STATIC ${CINOLIB_SOURCE_FILES})
	target_compile_definitions(cinolib ${CINOLIB_ACCESS} CINO_STATIC_LIB)
endif()

target_include_directories(cinolib ${CINOLIB_ACCESS} "${cinolib_DIR}/include" "${cinolib_DIR}/external/eigen")

# https://cliutils.gitlab.io/modern-cmake/chapters/features/cpp11.html
# (bugfix: cinoLib requires only C++11 but modern Boost requires C++14)
target_compile_features(cinolib ${CINOLIB_ACCESS} cxx_std_14)
set_target_properties(cinolib PROPERTIES CXX_EXTENSIONS OFF)

#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
# OPTIONAL MODULES ::::::::::::::::::::::::::::::::::::::::::::::::::::::
#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

# https://cmake.org/cmake/help/latest/policy/CMP0077.html
cmake_policy(SET CMP0077 NEW)

option(CINOLIB_USES_OPENGL_GLFW_IMGUI "Use OpenGL, GLFW and ImGui" OFF)
option(CINOLIB_USES_TETGEN            "Use Tetgen"                 OFF)
option(CINOLIB_USES_TRIANGLE          "Use Triangle"               OFF)
option(CINOLIB_USES_EXACT_PREDICATES  "Use Exact Predicates"       OFF)
option(CINOLIB_USES_GRAPH_CUT         "Use Graph Cut"              OFF)
option(CINOLIB_USES_BOOST             "Use Boost"                  OFF)
option(CINOLIB_USES_VTK               "Use VTK"                    OFF)

#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

if(CINOLIB_USES_OPENGL_GLFW_IMGUI)
	message("CINOLIB OPTIONAL MODULES: OpenGL, GLFW, ImGui")
	find_package(OpenGL)
	if(OpenGL_FOUND)
		target_link_libraries(cinolib ${CINOLIB_ACCESS} OpenGL::GL)
		add_subdirectory(${cinolib_DIR}/external/imgui imgui)
		target_link_libraries(cinolib ${CINOLIB_ACCESS} imgui)
		target_compile_definitions(cinolib ${CINOLIB_ACCESS} CINOLIB_USES_OPENGL_GLFW_IMGUI GL_SILENCE_DEPRECATION)
	else()
		message("Could not find OpenGL!")
	set(CINOLIB_USES_OPENGL_GLFW_IMGUI OFF)
	endif()
endif()

#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

if(CINOLIB_USES_TETGEN)
	message("CINOLIB OPTIONAL MODULE: Tetgen")
	FetchContent_Declare(tetgen GIT_REPOSITORY "https://github.com/cinolib-dev-team/tetgen.git")
	FetchContent_MakeAvailable(tetgen)
	target_compile_definitions(cinolib ${CINOLIB_ACCESS} CINOLIB_USES_TETGEN)
	target_compile_definitions(cinolib ${CINOLIB_ACCESS} TETLIBRARY)
	target_compile_definitions(cinolib ${CINOLIB_ACCESS} _CRT_SECURE_NO_WARNINGS) # disable MSVC unsafe warnings
	target_include_directories(cinolib ${CINOLIB_ACCESS} ${tetgen_SOURCE_DIR})
	target_link_libraries(cinolib ${CINOLIB_ACCESS} tetgen)
endif()

#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

if(CINOLIB_USES_TRIANGLE)
	message("CINOLIB OPTIONAL MODULE: Triangle")
	FetchContent_Declare(triangle GIT_REPOSITORY "https://github.com/cinolib-dev-team/triangle.git")
	FetchContent_MakeAvailable(triangle)
	target_compile_definitions(cinolib ${CINOLIB_ACCESS} CINOLIB_USES_TRIANGLE)
	target_include_directories(cinolib ${CINOLIB_ACCESS} ${triangle_SOURCE_DIR})
	target_link_libraries(cinolib ${CINOLIB_ACCESS} triangle)
endif()

##::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

if(CINOLIB_USES_EXACT_PREDICATES)
	message("CINOLIB OPTIONAL MODULE: Exact Predicates")
	add_subdirectory(${cinolib_DIR}/external/predicates predicates)
	target_link_libraries(cinolib ${CINOLIB_ACCESS} predicates)
	target_compile_definitions(cinolib ${CINOLIB_ACCESS} CINOLIB_USES_EXACT_PREDICATES)
endif()

#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

if(CINOLIB_USES_GRAPH_CUT)
	message("CINOLIB OPTIONAL MODULE: Graph Cuts")
	FetchContent_Declare(graph_cuts GIT_REPOSITORY "https://github.com/cinolib-dev-team/GraphCuts.git")
	FetchContent_MakeAvailable(graph_cuts)
	target_compile_definitions(cinolib ${CINOLIB_ACCESS} CINOLIB_USES_GRAPH_CUT)
	target_link_libraries(cinolib ${CINOLIB_ACCESS} graph_cuts)
	target_include_directories(cinolib ${CINOLIB_ACCESS} ${graph_cuts_SOURCE_DIR})
endif()

#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

if(CINOLIB_USES_BOOST)
	message("CINOLIB OPTIONAL MODULE: Boost")
	find_package(Boost)
	if(Boost_FOUND)
		target_include_directories(cinolib ${CINOLIB_ACCESS} ${Boost_INCLUDE_DIRS})
		target_compile_definitions(cinolib ${CINOLIB_ACCESS} CINOLIB_USES_BOOST)
	else()
		message("Could not find Boost!")
	set(CINOLIB_USES_BOOST OFF)
	endif()
endif()

#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

if(CINOLIB_USES_VTK)
	message("CINOLIB OPTIONAL MODULE: VTK")
	find_package(VTK COMPONENTS IOGeometry IOImport IOExport)
	if(VTK_FOUND)
		# https://vtk.org/doc/nightly/html/md__builds_gitlab-kitware-sciviz-ci_Documentation_Doxygen_ModuleMigration.html
		target_link_libraries(cinolib ${CINOLIB_ACCESS} VTK::IOGeometry VTK::IOImport VTK::IOExport)
		vtk_module_autoinit(TARGETS cinolib MODULES VTK::IOGeometry VTK::IOImport VTK::IOExport)
		target_compile_definitions(cinolib ${CINOLIB_ACCESS} CINOLIB_USES_VTK)
	else()
		message("Could not find VTK!")
		set(CINOLIB_USES_VTK OFF)
	endif()
endif()

#::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
