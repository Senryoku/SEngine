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
IF(${PkgConfig_FOUND})
	pkg_search_module(GLFW REQUIRED glfw3)
	include_directories(${GLFW_INCLUDE_DIRS})
ELSE()
	SET(GLFW3_ROOT_DIR "../glfw" CACHE FILEPATH "GLFW3_ROOT_DIR")
	SET(GLFW3_INCLUDE_DIR ${GLFW3_ROOT_DIR}/include CACHE FILEPATH "GLFW3_INCLUDE_DIR")
	SET(GLFW3_LIBRARY_DIRS ${GLFW3_ROOT_DIR}/lib CACHE FILEPATH "GLFW3_LIBRARY_DIRS")
	INCLUDE_DIRECTORIES(${GLFW3_INCLUDE_DIR})
	LINK_DIRECTORIES(${GLFW3_LIBRARY_DIRS})
	ADD_DEFINITIONS(${GLFW3_DEFINITIONS})
ENDIF()

#GLM
SET(GLM_INCLUDE_DIR "../glm" CACHE FILEPATH "GLM_INCLUDE_DIR")
INCLUDE_DIRECTORIES(${GLM_INCLUDE_DIR})

#dear imgui
set(IMGUI_ROOT "ext/imgui")
INCLUDE_DIRECTORIES(${IMGUI_ROOT})
set(IMGUIGLFW_ROOT "ext/imgui-glfw")
INCLUDE_DIRECTORIES(${IMGUIGLFW_ROOT})
set(SOURCE_FILES ${SOURCE_FILES}
	${IMGUI_ROOT}/imgui.cpp
	${IMGUI_ROOT}/imgui_draw.cpp
	${IMGUI_ROOT}/imgui_dock.cpp
	${IMGUIGLFW_ROOT}/imgui_impl_glfw_gl3.cpp
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
		target_link_libraries(${Target} SEngine glfw3 ${GLFW_LIBRARIES} SenOGL)
		MESSAGE(STATUS "Adding rule for ${Target}.")
	endif(NOT TARGET ${Target})
endforeach(Exe)

add_custom_target(run
    COMMAND Tests
    DEPENDS Tests
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)

add_custom_target(doc
    COMMAND doxygen Doxyfile
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)
