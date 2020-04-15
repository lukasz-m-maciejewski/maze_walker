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
    square_rectangular_maze_proto)

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

#  configure_file(${CMAKE_SOURCE_DIR}/assets/cross.png ${CMAKE_BINARY_DIR}/assets/cross.png COPYONLY)
#  configure_file(${CMAKE_SOURCE_DIR}/assets/circle.png ${CMAKE_BINARY_DIR}/assets/circle.png COPYONLY)
