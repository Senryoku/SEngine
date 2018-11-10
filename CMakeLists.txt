cmake_minimum_required (VERSION 2.6)

cmake_policy(SET CMP0015 NEW)
cmake_policy(SET CMP0048 OLD)

project (SEngine)
set (SEngine_VERSION_MAJOR 2)
set (SEngine_VERSION_MINOR 0)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Libraries ------------------------------------------------------------------------

#OpenGL
FIND_PACKAGE(OpenGL REQUIRED)
#MESSAGE(STATUS "OpenGL Found ? ${OPENGL_FOUND}")
INCLUDE_DIRECTORIES(${OpenGL_INCLUDE_DIR})
LINK_DIRECTORIES(${OpenGL_LIBRARY_DIRS})
ADD_DEFINITIONS(${OpenGL_DEFINITIONS})

FIND_PACKAGE(PkgConfig)

SET(EXT_LIB_BUILD "extlib_build" CACHE FILEPATH "Where the external libraries will be build.")

include_directories("ext/")

#SENOGL
add_subdirectory("ext/SenOGL")
# This should not be necessary...
include_directories("ext/SenOGL/include")
include_directories("ext/SenOGL/include/shaders")
include_directories("ext/SenOGL/include/textures")
include_directories("ext/SenOGL/external/stb_image")
include_directories("ext/SenOGL/external/gl3w/include")

#GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW3_ROOT_DIR "ext/glfw" CACHE FILEPATH "GLFW3_ROOT_DIR")
set(GLFW3_INCLUDE_DIR ${GLFW3_ROOT_DIR}/include CACHE FILEPATH "GLFW3_INCLUDE_DIR")
add_subdirectory(${GLFW3_ROOT_DIR})
include_directories(${GLFW3_INCLUDE_DIR})

#GLM
SET(GLM_INCLUDE_DIR "ext/glm" CACHE FILEPATH "GLM_INCLUDE_DIR")
INCLUDE_DIRECTORIES(${GLM_INCLUDE_DIR})

#dear imgui
set(IMGUI_ROOT "ext/imgui")
INCLUDE_DIRECTORIES(${IMGUI_ROOT})
set(IMGUIGLFW_ROOT "ext/imgui/examples/")
INCLUDE_DIRECTORIES(${IMGUIGLFW_ROOT})
set(SOURCE_FILES ${SOURCE_FILES}
	${IMGUI_ROOT}/imgui.cpp
	${IMGUI_ROOT}/imgui_widgets.cpp
	${IMGUI_ROOT}/imgui_draw.cpp
	${IMGUIGLFW_ROOT}/imgui_impl_opengl3.cpp
	${IMGUIGLFW_ROOT}/imgui_impl_glfw.cpp
)

set(TINYOBJLOADER_ROOT "ext/tinyobjloader")
INCLUDE_DIRECTORIES(${TINYOBJLOADER_ROOT})
set(SOURCE_FILES ${SOURCE_FILES}
	${TINYOBJLOADER_ROOT}/tiny_obj_loader.cc
)

#json
include_directories("ext/json")

# -----------------------------------------------------------------------------------

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
        LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

MACRO(REC_SOURCE ParentDir)
	#MESSAGE(STATUS "SubDir: ${ParentDir}")
	INCLUDE_DIRECTORIES(${ParentDir})
	AUX_SOURCE_DIRECTORY(${ParentDir} SOURCE_FILES)
	
	SUBDIRLIST(SUBDIRECTORIES ${ParentDir})

	foreach(DIR ${SUBDIRECTORIES})
		REC_SOURCE(${ParentDir}/${DIR})
		
	endforeach(DIR)
ENDMACRO()

REC_SOURCE(${CMAKE_SOURCE_DIR}/src)

AUX_SOURCE_DIRECTORY(exe/ EXECUTABLES)

list(APPEND CMAKE_CXX_FLAGS "-std=c++17 -fopenmp -Wall -Wfatal-errors ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_library(SEngine STATIC ${SOURCE_FILES})

foreach(Exe ${EXECUTABLES})
	get_filename_component(Target ${Exe} NAME_WE [CACHE])
	if(NOT TARGET ${Target})
		add_executable(${Target} ${Exe})
		# std::filesystem
		if(WIN32)
			target_link_libraries(${Target} stdc++fs KtmW32)
		else()
			target_link_libraries(${Target} stdc++fs)
		endif()
		target_link_libraries(${Target} SEngine glfw ${GLFW_LIBRARIES} SenOGL)
		MESSAGE(STATUS "Adding rule for ${Target}.")
	endif(NOT TARGET ${Target})
endforeach(Exe)

add_custom_target(run
    COMMAND Tests
    DEPENDS Tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_target(doc
    COMMAND doxygen Doxyfile
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
