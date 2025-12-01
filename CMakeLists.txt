# StateSnap - Orbiter Autosave Module
# CMake build configuration

cmake_minimum_required(VERSION 3.20)
project(StateSnap VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to build standalone (outside Orbiter source tree)
option(STATESNAP_STANDALONE "Build standalone without Orbiter source tree" OFF)

if(STATESNAP_STANDALONE)
    # Standalone build - requires ORBITER_SDK_DIR to be set
    if(NOT DEFINED ORBITER_SDK_DIR)
        message(FATAL_ERROR "ORBITER_SDK_DIR must be set when building standalone. "
                "Point it to the Orbiter SDK installation directory.")
    endif()

    set(ORBITER_SOURCE_SDK_INCLUDE_DIR "${ORBITER_SDK_DIR}/include")
    set(ORBITER_SDK_LIB_DIR "${ORBITER_SDK_DIR}/lib")

    # Find the SDK library
    find_library(ORBITER_SDK_LIB
        NAMES Orbiter Orbitersdk orbiter
        PATHS "${ORBITER_SDK_LIB_DIR}"
        REQUIRED
    )

    message(STATUS "Orbiter SDK include: ${ORBITER_SOURCE_SDK_INCLUDE_DIR}")
    message(STATUS "Orbiter SDK library: ${ORBITER_SDK_LIB}")
else()
    # Building as part of Orbiter source tree
    # These variables should be set by parent CMakeLists.txt
    if(NOT DEFINED ORBITER_SOURCE_SDK_INCLUDE_DIR)
        message(FATAL_ERROR "ORBITER_SOURCE_SDK_INCLUDE_DIR not defined. "
                "Either build as part of Orbiter or set STATESNAP_STANDALONE=ON")
    endif()
endif()

# MSVC-specific settings
if(MSVC)
    add_compile_options(/we4311 /DNOMINMAX)
    # Multi-processor compilation
    if(NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
        add_compile_options(/MP)
    endif()
endif()

# Output directory
if(DEFINED ORBITER_BINARY_PLUGIN_DIR)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ORBITER_BINARY_PLUGIN_DIR})
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Modules/Plugin)
endif()

# Create shared library (DLL)
add_library(StateSnap SHARED
    StateSnap.cpp
    StateSnap.h
    StateSnapDlg.cpp
    StateSnapDlg.h
    Resources.rc
    resource.h
)

# Include paths
target_include_directories(StateSnap
    PUBLIC ${ORBITER_SOURCE_SDK_INCLUDE_DIR}
)

# Link libraries
if(STATESNAP_STANDALONE)
    target_link_libraries(StateSnap
        ${ORBITER_SDK_LIB}
    )
else()
    target_link_libraries(StateSnap
        ${ORBITER_LIB}
        ${ORBITER_SDK_LIB}
    )
    # Dependencies when building with Orbiter
    if(TARGET ${OrbiterTgt})
        add_dependencies(StateSnap ${OrbiterTgt})
    endif()
    if(TARGET Orbitersdk)
        add_dependencies(StateSnap Orbitersdk)
    endif()
endif()

# Set target properties
set_target_properties(StateSnap
    PROPERTIES
    FOLDER Modules/Extern
    OUTPUT_NAME "StateSnap"
)

# Installation
if(DEFINED ORBITER_INSTALL_PLUGIN_DIR)
    install(TARGETS StateSnap
        RUNTIME DESTINATION ${ORBITER_INSTALL_PLUGIN_DIR}
    )
else()
    install(TARGETS StateSnap
        RUNTIME DESTINATION Modules/Plugin
    )
endif()

# Also install license
install(FILES LICENSE
    DESTINATION Doc/StateSnap
    OPTIONAL
)
