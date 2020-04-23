# Generic test that uses conan libs

find_package(Protobuf REQUIRED)

add_executable(game main.cpp)
target_link_libraries(
  game
  PRIVATE
    solarized_colors
    project_options
    project_warnings
    CONAN_PKG::docopt.cpp
    CONAN_PKG::imgui-sfml
    CONAN_PKG::fmt
    CONAN_PKG::spdlog
    CONAN_PKG::Outcome
    square_rectangular_maze_proto
    util
    mazegen
    )

add_library(solarized_colors
  solarized.cpp)

target_link_libraries(
    solarized_colors
    PRIVATE
    CONAN_PKG::imgui-sfml
    )

protobuf_generate_cpp(MAZE_PROTO_SRCS MAZE_PROTO_HDRS square_rectangular_maze.proto)

add_library(square_rectangular_maze_proto ${MAZE_PROTO_SRCS})
target_include_directories(square_rectangular_maze_proto
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
  PRIVATE ${Protobuf_INCLUDE_DIRS})
target_link_libraries(square_rectangular_maze_proto ${Protobuf_LIBRARIES})

add_library(util util/grid.cpp)
target_include_directories(util PUBLIC util)
target_link_libraries(
  util
  PUBLIC
    CONAN_PKG::Outcome
  )

add_executable(util_test util/grid_test.cpp)
target_link_libraries(util_test PRIVATE util catch_main project_warnings project_options)
add_test(NAME util_test COMMAND util_test)

add_library(mazegen mazegen_growing_tree.cpp mazegen_growing_tree.hpp)
target_link_libraries(
  mazegen
  PUBLIC
    square_rectangular_maze_proto
    CONAN_PKG::Outcome
  PRIVATE
    util
    project_warnings
    project_options
  )

add_executable(mazegen_test mazegen_growing_tree.test.cpp)
target_link_libraries(mazegen_test PRIVATE mazegen catch_main project_warnings project_options)
add_test(NAME mazegen_test COMMAND mazegen_test)

configure_file(
  ${CMAKE_SOURCE_DIR}/assets/roadTextures.png
  ${CMAKE_BINARY_DIR}/assets/roadTextures.png
  COPYONLY)
#  configure_file(${CMAKE_SOURCE_DIR}/assets/circle.png ${CMAKE_BINARY_DIR}/assets/circle.png COPYONLY)
