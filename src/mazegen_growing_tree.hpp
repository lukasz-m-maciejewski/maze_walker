#pragma once

#include <outcome.hpp>

#include "square_rectangular_maze.pb.h"

namespace outcome = OUTCOME_V2_NAMESPACE;

namespace maze_walker {

outcome::result<SquareRectangularMazeData> GenerateMaze(int num_cols,
                                                        int num_rows);

outcome::result<std::vector<SquareRectangularMazeData>> GenerateMazeWithSteps(
    int num_cols, int num_rows);

}  // namespace maze_walker
