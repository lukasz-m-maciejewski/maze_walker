#include "mazegen_growing_tree.hpp"

#include <catch2/catch.hpp>

namespace maze_walker {
TEST_CASE("Generate small maze", "[maze]") {
  SquareRectangularMazeData data = GenerateMaze(3, 3).value();
  std::cout << data.DebugString();
  REQUIRE(data.num_cols() == 3);
  REQUIRE(data.num_rows() == 3);

}
}  // namespace util
