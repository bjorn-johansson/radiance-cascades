cmake_minimum_required(VERSION 3.13.0...3.19.3)
project(radiance-cascades VERSION 1.0.0 DESCRIPTION "TNM061 Project: Radiance Cascades" LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(enable_warnings target)
target_compile_options(${target} PUBLIC 
$<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
$<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -Wconversion>
$<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wconversion>
$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wconversion>
)
endfunction()

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

set(HEADER_FILES
	Rotator.hpp
	Shader.hpp
	Texture.hpp
	TriangleSoup.hpp
	Utilities.hpp
)

set(SOURCE_FILES
	GLMain.cpp
	Rotator.cpp
	Shader.cpp
	Texture.cpp
	TriangleSoup.cpp
	Utilities.cpp
)

add_executable(radiance-cascades ${SOURCE_FILES} ${HEADER_FILES})
enable_warnings(radiance-cascades)

if(MSVC AND TARGET radiance-cascades)
	set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT radiance-cascades)
	set_property(TARGET radiance-cascades PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
elseif(CMAKE_GENERATOR MATCHES "Xcode"
	AND ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.15.0"
	AND TARGET radiance-cascades)
	set_target_properties(radiance-cascades PROPERTIES
		XCODE_GENERATE_SCHEME TRUE
		XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	)
endif()

target_compile_definitions(radiance-cascades PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)

target_link_libraries(radiance-cascades PRIVATE OpenGL::GL glfw)

option(RC_USE_EXTERNAL_GLEW "GLEW is provided externally" OFF)
# Set CMake to prefere Vendor gl libraries rather than legacy, fixes warning on some unix systems
if(NOT RC_USE_EXTERNAL_GLEW)
    set(OpenGL_GL_PREFERENCE GLVND) 
	add_subdirectory(glew)
	target_link_libraries(radiance-cascades PUBLIC RC::GLEW)
else()
	find_package(GLEW REQUIRED)
	target_link_libraries(radiance-cascades PUBLIC GLEW::GLEW)
endif()


if(EXISTS "${CMAKE_SOURCE_DIR}/tracy/public")
    message(STATUS "Tracy found at ${CMAKE_SOURCE_DIR}/tracy/public")
    
    include_directories("${CMAKE_SOURCE_DIR}/tracy/public/tracy")

    # Enable Tracy profiling
    add_definitions(-DTRACY_ENABLE)

    # Add TracyClient.cpp as a source file
    set(TRACY_SOURCES "tracy/public/TracyClient.cpp")

    # Include TracyClient.cpp in your project target
    target_sources(radiance-cascades PRIVATE ${TRACY_SOURCES})
else()
     message(STATUS "Tracy not found! It should be at:\n ${CMAKE_SOURCE_DIR}/RadianceCascades/tracy/public/tracy \n")
endif()