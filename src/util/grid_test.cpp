#include "grid.hpp"

#include <catch2/catch.hpp>

namespace util {
TEST_CASE("Instantiating grid", "[grid]") {
  Grid<int> g = Grid<int>::Make(3, 3).value();
  REQUIRE(g.num_cols() == 3);
  REQUIRE(g.num_rows() == 3);

  SECTION("Accesing elements") {
    auto location = g.MakeLocation(1, 2).value();
    REQUIRE(g.at(location) == 0);
    g.at(location) = 10;
    REQUIRE(g.at(location) == 10);
  }
}
}  // namespace util
