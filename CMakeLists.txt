cmake_minimum_required(VERSION 3.22)
project(chemistry-bot VERSION 0.1 DESCRIPTION "A discord bot for not missing homework :skull:")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Create executable
add_executable(
    ${PROJECT_NAME}
    src/main.cpp
)

# Find DPP
find_package(DPP REQUIRED)

# Link DPP
target_link_libraries(
    ${PROJECT_NAME}
    ${DPP_LIBRARIES}
)

# Include DPP
target_include_directories(
    ${PROJECT_NAME} PRIVATE
    ${DPP_INCLUDE_DIRS}
)

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20 
    CXX_STANDARD_REQUIRED YES
)
