cmake_minimum_required(VERSION 3.15)
project(calculator_project)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the build type to Debug
set(CMAKE_BUILD_TYPE Debug)


# Find packages
find_package(CycloneDDS REQUIRED)
find_package(CycloneDDS-CXX REQUIRED)
find_package(flatbuffers REQUIRED)

# Generate C++ files from IDL
get_filename_component(DLL_PATH ${_idlcxx_shared_lib} DIRECTORY)
set(IDL_INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/idl/calculator.idl")
set(IDL_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/idl_out")
set(IDL_HDRS ${IDL_OUT_DIR}/calculator.hpp)
set(IDL_SRCS ${IDL_OUT_DIR}/calculator.cpp)
add_custom_command(
    OUTPUT ${IDL_SRCS} ${IDL_HDRS}
    COMMAND ${CMAKE_COMMAND} -E env "PATH=${DLL_PATH}" ${_idlc_executable} 
    ARGS -l c++ -o ${IDL_OUT_DIR} ${IDL_INPUT_FILE}
    DEPENDS ${IDL_INPUT_FILE}
    WORKING_DIRECTORY ${IDL_OUT_DIR}
    COMMENT "Generating C++ files from IDL"
)
add_custom_target(idl_files DEPENDS ${IDL_HDRS} ${IDL_SRCS})
include_directories(${IDL_OUT_DIR})

# Add executable targets
add_executable(calculator_server src/calculator_server.cpp  ${IDL_SRCS})
add_executable(calculator_client src/calculator_client.cpp  ${IDL_SRCS})

# Ensure IDL files are generated before building executables
add_dependencies(calculator_server idl_files)
add_dependencies(calculator_client idl_files)

# Link libraries
target_link_libraries(calculator_server ${CycloneDDS_LIBRARIES} ${CycloneDDS-CXX_LIBRARIES})
target_link_libraries(calculator_client ${CycloneDDS_LIBRARIES} ${CycloneDDS-CXX_LIBRARIES})

# Add include directories from CycloneDDS package
target_include_directories(calculator_server PRIVATE ${CycloneDDS_INCLUDE_DIRS} ${CycloneDDS-CXX_INCLUDE_DIRS})
target_include_directories(calculator_client PRIVATE ${CycloneDDS_INCLUDE_DIRS} ${CycloneDDS-CXX_INCLUDE_DIRS})