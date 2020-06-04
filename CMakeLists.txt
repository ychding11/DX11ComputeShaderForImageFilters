cmake_minimum_required (VERSION 3.6)

set(PROJECT_NAME ImageLab)

project (${PROJECT_NAME})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WINDOWS)
  set(GUI_TYPE WIN32)
elseif(MACOSX)
  set(GUI_TYPE MACOSX_BUNDLE)
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "Configuration types" FORCE)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

set(ENABLE_HIDECONSOLE_BUILD TRUE CACHE BOOL "TRUE to hide console on Windows.")

set(VERBOSE_MODE FALSE CACHE BOOL "TRUE to output verbose info.")

set(_IMAGE_LOCATION_ ${CMAKE_CURRENT_SOURCE_DIR}/images/ CACHE STRING "Image Location" FORCE)

set(_IMAGE_EFFECT_LOCATION_ ${CMAKE_CURRENT_SOURCE_DIR}/effects/ CACHE STRING "Image Effect Location" FORCE)

# Add UGS to build 
set(UGS_BUILD_SAMPLES OFF CACHE BOOL "Build UGS Samples")
add_subdirectory(UGS)
message(STATUS "####################################### Build 'UGS' done #######################################")

add_compile_definitions(UGS_LOCATION="${_UGS_LOCATION_}")
add_compile_definitions(UGS_SHADER_LOCATION="${_UGS_SHADER_LOCATION_}")

add_compile_definitions(IMAGE_LOCATION="${_IMAGE_LOCATION_}")
add_compile_definitions(IMAGE_EFFECT_LOCATION="${_IMAGE_EFFECT_LOCATION_}")


file(GLOB SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

file(GLOB EXT_FILES
)

file(GLOB IMAGE_SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/effects/*.hlsl
)

# group_files defined in UGS cmakelists.txt
group_files("${SRC_FILES}")
group_files("${EXT_FILES}")
group_files("${IMAGE_SHADER_FILES}")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/UGS/include
  ${CMAKE_CURRENT_SOURCE_DIR}/UGS/framework/thirdparty/imgui/
)

link_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/bin
)

add_compile_definitions(UGS_LOCATION="${_UGS_LOCATION_}")
add_compile_definitions(UGS_SHADER_LOCATION="${_UGS_SHADER_LOCATION_}")
add_compile_definitions(IMAGE_SHADER_LOCATION="${_IMAGE_SHADER_LOCATION_}")

if (VERBOSE_MODE)
	# list include directory for debug
	get_property(includes DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
	foreach(dir ${includes})
		message(STATUS "${PROJECT_NAME} include dir='${dir}'")
	endforeach()

	# list cmake module path 
	message(STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")
endif(VERBOSE_MODE)

function(compile_effect shaderfile dependencies)
	file(RELATIVE_PATH name ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${shaderfile})
	set(tempfile ${UGS_BINARY_DIR}/temp/${name}.temp)
	#message(STATUS "shaderfile='${shaderfile}' tempfile='${tempfile}' ")
	add_custom_command(
		OUTPUT ${tempfile}
		COMMAND ${FXC} /T cs_5_0 /E CSMain /Fo ${tempfile}  ${shaderfile}
		MAIN_DEPENDENCY ${shaderfile}
		DEPENDS ${dependencies}
	)
endfunction(compile_effect)

foreach(shaderfile ${IMAGE_SHADER_FILES})
	compile_effect(${shaderfile} "${shaderDependencies}")
endforeach()

#######################################################################################
## preprocess macro definition
#######################################################################################
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-DBX_CONFIG_ENABLE_MSVC_LEVEL4_WARNINGS=1)
add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
add_definitions(-DUSE_DL_PREFIX)

if(MSVC)
	add_definitions(-DWIN32)
	add_definitions(-D_WIN32)
	add_definitions(-DUNICODE)
	add_definitions(-D_UNICODE)
	#add_definitions(-DNOMINMAX)
endif(MSVC)

# Check compiler flag
if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++17" _cpp_17_flag_supported)
    if (_cpp_17_flag_supported)
		message(STATUS "C++ 17 is suppoted in VS")
        add_compile_options("/std:c++17")
    endif()
endif()


if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

#find_package(DirectX)

if(VERBOSE_MODE)
	message(STATUS "CMAKE_CXX_FLAGS='${CMAKE_CXX_FLAGS}' ")
	message(STATUS "LINK_OPTIONS='${LINK_OPTIONS}' ")
	message(STATUS "UGS_SHADER_LOCATION='${UGS_SHADER_LOCATION}' ")
	message(STATUS "ALL_UGS_LIBS='${ALL_UGS_LIBS}' ")
endif(VERBOSE_MODE)

# Image Effects based on ugs
set(target_name "ImageEffects")
add_executable (${target_name} ${SRC_FILES} ${IMAGE_SHADER_FILES})
target_link_libraries( ${target_name} ${ALL_UGS_LIBS} )
set_target_properties( ${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin )
set_target_properties( ${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin )
set_target_properties( ${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/bin )
set_target_properties( ${target_name} PROPERTIES DEBUG_POSTFIX "_d")
set_target_properties( ${target_name} PROPERTIES RELWITHDEBINFO_POSTFIX "RelWithDebInfo")
set_target_properties( ${target_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Hide the console window in visual studio projects
if(ENABLE_HIDECONSOLE_BUILD)
	if(MSVC)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
		set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
		set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
	endif()
endif()
