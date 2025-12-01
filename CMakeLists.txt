# StateSnap - Orbiter Autosave Module
# CMake build configuration
#
# Build as part of Orbiter source tree:
#   Copy to Orbiter/Extern/StateSnap and add to Extern/CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
project(StateSnap VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Verify we're building as part of Orbiter
if(NOT DEFINED ORBITER_SOURCE_SDK_INCLUDE_DIR)
    message(FATAL_ERROR
        "StateSnap must be built as part of the Orbiter source tree.\n"
        "Copy this directory to Orbiter/Extern/StateSnap and add:\n"
        "  add_subdirectory(StateSnap)\n"
        "to Orbiter/Extern/CMakeLists.txt")
endif()

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ORBITER_BINARY_PLUGIN_DIR})

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
target_link_libraries(StateSnap
    ${ORBITER_LIB}
    ${ORBITER_SDK_LIB}
)

# Dependencies
add_dependencies(StateSnap
    ${OrbiterTgt}
    Orbitersdk
)

# Set target properties
set_target_properties(StateSnap
    PROPERTIES
    FOLDER Modules/Extern
)

# Installation
install(TARGETS StateSnap
    RUNTIME DESTINATION ${ORBITER_INSTALL_PLUGIN_DIR}
)

install(FILES LICENSE
    DESTINATION ${ORBITER_INSTALL_DOC_DIR}/StateSnap
    OPTIONAL
)
