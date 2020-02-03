cmake_minimum_required (VERSION 3.6)
project (ImageEffects)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WINDOWS)
  set(GUI_TYPE WIN32)
elseif(MACOSX)
  set(GUI_TYPE MACOSX_BUNDLE)
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "Configuration types" FORCE)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

############################################################################
# Custom Lib & Link directory
############################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/")
link_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/framework/thirdparty/Assimp-3.1.1/lib
)

#######################################################################################
## Check compiler flag
#######################################################################################
if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++17" _cpp_17_flag_supported)
    if (_cpp_17_flag_supported)
		message(STATUS "C++ 17 is suppoted in VS")
        add_compile_options("/std:c++17")
    endif()
endif()


file(GLOB SRC_FILES
    ${CMAKE_SOURCE_DIR}/Source/*.h
    ${CMAKE_SOURCE_DIR}/Source/*.cpp
)

file(GLOB EXT_FILES
)

file(GLOB FRAMEWORK_FILES
    ${CMAKE_SOURCE_DIR}/framework/*.h
	${CMAKE_SOURCE_DIR}/framework/*.cpp
	${CMAKE_SOURCE_DIR}/framework/d3d/*.h
	${CMAKE_SOURCE_DIR}/framework/d3d/*.cpp
	${CMAKE_SOURCE_DIR}/framework/camera/*.h
	${CMAKE_SOURCE_DIR}/framework/camera/*.cpp
	${CMAKE_SOURCE_DIR}/framework/core/*.h
	${CMAKE_SOURCE_DIR}/framework/core/*.cpp
	${CMAKE_SOURCE_DIR}/framework/io/*.h
	${CMAKE_SOURCE_DIR}/framework/io/*.cpp
	${CMAKE_SOURCE_DIR}/framework/utils/*.h
	${CMAKE_SOURCE_DIR}/framework/utils/*.cpp
	${CMAKE_SOURCE_DIR}/framework/GHI/*.h
	${CMAKE_SOURCE_DIR}/framework/GHI/*.cpp 
	${CMAKE_SOURCE_DIR}/framework/thirdparty/*.h
	${CMAKE_SOURCE_DIR}/framework/thirdparty/imgui/*.h
	${CMAKE_SOURCE_DIR}/framework/thirdparty/imgui/*.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/Source/
  ${CMAKE_SOURCE_DIR}/framework/thirdparty/
  ${CMAKE_SOURCE_DIR}/framework/thirdparty/imgui/
  ${CMAKE_SOURCE_DIR}/framework/thirdparty/Assimp-3.1.1/include
  ${CMAKE_SOURCE_DIR}/framework/
  ${CMAKE_SOURCE_DIR}/framework/d3d
  ${CMAKE_SOURCE_DIR}/framework/GHI
  ${CMAKE_SOURCE_DIR}/framework/utils
  ${CMAKE_SOURCE_DIR}/framework/core
)

#######################################################################################
## Print include directory for debug
#######################################################################################
get_property(includes DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${includes})
    message(STATUS "include dir='${dir}'")
endforeach()

###########################################################################
# custom-defined function
# set target property
# reference https://cmake.org/cmake/help/v3.0/manual/cmake-properties.7.html 
###########################################################################
function(my_default_target_property target_name)
  #message(STATUS "ARGC=${ARGC} target_name='${target_name}' ")
  if (NOT ${ARGC} EQUAL 1)
    message(STATUS "ARGC is NOT expected.") 
  endif()
  if (NOT ${ARGV0} STREQUAL ${target_name})
    message(STATUS "ARGV0 is NOT expected. ARGV0='${ARGV0}' target_name='${target_name}'") 
  endif()  
 
  set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin )
  set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin )
  set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin )
  set_target_properties(${target_name} PROPERTIES DEBUG_POSTFIX "_d")
  set_target_properties(${target_name} PROPERTIES RELWITHDEBINFO_POSTFIX "RelWithDebInfo")
  set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
endfunction()

#######################################################################################
## macro definition
#######################################################################################
macro(group_src_files src_files target)
	foreach(f ${src_files})
		#message(STATUS "check parameters original source file :${f}")
	endforeach()
	#message(STATUS "check parameters new group name :${target}")
    
    foreach(f ${src_files})
		# Get the path of the file relative to ${DIRECTORY},
		# then alter it (not compulsory)
		file(RELATIVE_PATH SRCGR ${CMAKE_SOURCE_DIR} ${f})
		set(SRCGR "${target}/${SRCGR}")
		
		# Extract the folder, ie remove the filename part
		string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})

		# Source_group expects \\ (double antislash), not / (slash)
		string(REPLACE / \\ SRCGR ${SRCGR})
		source_group("${SRCGR}" FILES ${f})
		#message(STATUS "New group name :${SRCGR}")
	endforeach()
endmacro()

group_src_files("${SRC_FILES}" "ImageEffects")
group_src_files("${EXT_FILES}" "External")
group_src_files("${FRAMEWORK_FILES}" "UGS")
#######################################################################################



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



if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

find_package(DirectX)

#set(LINK_OPTIONS " ")
message(STATUS "LINK_OPTIONS='${LINK_OPTIONS}' ")
message(STATUS "CMAKE_CXX_FLAGS='${CMAKE_CXX_FLAGS}' ")

###########################################################################
# ugs libraries and executables
###########################################################################
ADD_LIBRARY ( ugs STATIC  ${FRAMEWORK_FILES}  ${EXT_FILES} )

if (WIN32)
  # Avoid a name clash when building on Visual Studio
  set_target_properties ( ugs PROPERTIES OUTPUT_NAME libugs )
endif()

set(ALL_UGS_LIBS
  ugs
  ${DirectX_D3D11_LIBRARY} 
  ${DirectX_D3D11_COMPILER} 
  assimp.lib
  )


# Demos based on ugs
###########################################################################
set(EXE_NAME "ImageEffects")
ADD_EXECUTABLE ( ${EXE_NAME} ${SRC_FILES})
TARGET_LINK_LIBRARIES ( ${EXE_NAME} ${ALL_UGS_LIBS} )
my_default_target_property(${EXE_NAME})

#--------------------------------------------------------------------
# Hide the console window in visual studio projects
#--------------------------------------------------------------------
set(ENABLE_HIDECONSOLE_BUILD TRUE CACHE BOOL "TRUE to hide console on Windows.")
if(ENABLE_HIDECONSOLE_BUILD)
	if(MSVC)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	endif()
endif()

if(WINDOWS)
	set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
	set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
endif()

if(ENABLE_HIDECONSOLE_BUILD)
	message(STATUS "Console is hidden")
else()
	message(STATUS "Console is visible")
endif()

