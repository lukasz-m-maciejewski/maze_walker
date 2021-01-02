#include "mazegen_growing_tree.hpp"

#include <random>

#include "grid.hpp"

namespace maze_walker {
namespace {

enum class State {
  New,
  Active,
  Inactive,
};

struct Cell {
  State state;
  bool wall_down;
  bool wall_right;

  Cell() : state{State::New}, wall_down{true}, wall_right{true} {}
};

using Loc = util::Grid<Cell>::Location;

bool has_north_wall(const util::Grid<Cell>& grid,
                    const util::Grid<Cell>::Location& loc) {
  if (loc.row() == 0) return true;
  const auto one_up = grid.MakeLocation(loc.row() - 1, loc.col()).value();
  return grid.at(one_up).wall_down;
}

bool has_east_wall(const util::Grid<Cell>& grid,
                   const util::Grid<Cell>::Location& loc) {
  return grid.at(loc).wall_right;
}

bool has_south_wall(const util::Grid<Cell>& grid,
                    const util::Grid<Cell>::Location& loc) {
  return grid.at(loc).wall_down;
}

bool has_west_wall(const util::Grid<Cell>& grid,
                   const util::Grid<Cell>::Location& loc) {
  if (loc.col() == 0) return true;
  const auto one_left = grid.MakeLocation(loc.row(), loc.col() - 1).value();
  return grid.at(one_left).wall_right;
}

bool is_directly_left_of(const Loc& x, const Loc& y) {
  return x.row() == y.row() && x.col() - 1 == y.col();
}

bool is_directly_right_of(const Loc& x, const Loc& y) {
  return x.row() == y.row() && x.col() + 1 == y.col();
}

bool is_directly_above(const Loc& x, const Loc& y) {
  return x.row() - 1 == y.row() && x.col() == y.col();
}

bool is_directly_below(const Loc& x, const Loc& y) {
  return x.row() + 1 == y.row() && x.col() == y.col();
}

outcome::result<SquareRectangularMazeData> grid_to_maze(
    const util::Grid<Cell>& grid) {
  SquareRectangularMazeData maze;
  maze.set_num_cols(grid.num_cols());
  maze.set_num_rows(grid.num_rows());

  for (int row = 0; row < grid.num_rows(); ++row) {
    for (int col = 0; col < grid.num_cols(); ++col) {
      const auto loc = OUTCOME_TRYX(grid.MakeLocation(row, col));

      auto* cell = maze.add_walls();
      cell->set_n(has_north_wall(grid, loc));
      cell->set_e(has_east_wall(grid, loc));
      cell->set_s(has_south_wall(grid, loc));
      cell->set_w(has_west_wall(grid, loc));
    }
  }

  return maze;
}

int random_int_from_range(int a, int b) {
  using UID = std::uniform_int_distribution<>;
  static std::default_random_engine gen{std::random_device{}()};
  return UID{a, b}(gen);
}

Loc random_location(const util::Grid<Cell>& grid) {
  const int row_idx = random_int_from_range(0, grid.num_rows() - 1);
  const int col_idx = random_int_from_range(0, grid.num_cols() - 1);
  return grid.MakeLocation(row_idx, col_idx).value();
}

bool not_finished([[maybe_unused]] const util::Grid<Cell>& grid,
                  const std::vector<Loc>& active_set) {
  return not active_set.empty();
}

std::vector<Loc>::iterator next_loc(std::vector<Loc>& active_set) {
  return active_set.end() - 1;
}

template <typename Pred>
std::vector<Loc> neighbours_for(const util::Grid<Cell>& grid, const Loc& loc,
                                Pred pred) {
  std::vector<Loc> neighbours;
  neighbours.reserve(4);

  if (loc.row() > 0) {
    auto n_loc = grid.MakeLocation(loc.row() - 1, loc.col()).value();
    if (pred(grid.at(n_loc))) neighbours.push_back(n_loc);
  }

  if (loc.col() > 0) {
    auto n_loc = grid.MakeLocation(loc.row(), loc.col() - 1).value();
    if (pred(grid.at(n_loc))) neighbours.push_back(n_loc);
  }

  if (loc.row() < grid.num_rows() - 1) {
    auto n_loc = grid.MakeLocation(loc.row() + 1, loc.col()).value();
    if (pred(grid.at(n_loc))) neighbours.push_back(n_loc);
  }

  if (loc.col() < grid.num_cols() - 1) {
    auto n_loc = grid.MakeLocation(loc.row(), loc.col() + 1).value();
    if (pred(grid.at(n_loc))) neighbours.push_back(n_loc);
  }

  return neighbours;
}

bool is_new(const Cell& cell) { return cell.state == State::New; }
// bool is_not_new(const Cell& cell) { return cell.state != State::New; }

bool has_no_new_neighbours(const util::Grid<Cell>& grid, const Loc& loc) {
  const auto new_neighbours = neighbours_for(grid, loc, is_new);
  return new_neighbours.empty();
}

void remove_location(std::vector<Loc>& active_set,
                     std::vector<Loc>::iterator elem) {
  active_set.erase(elem);
}

void merge_random_neighbour(util::Grid<Cell>& grid,
                            std::vector<Loc>& active_set, const Loc loc) {
  std::vector<Loc> new_neighbours = neighbours_for(grid, loc, is_new);
  auto idx = static_cast<std::size_t>(
      random_int_from_range(0, static_cast<int>(new_neighbours.size()) - 1));
  const Loc& random_neighbour_loc = new_neighbours[idx];
  active_set.push_back(random_neighbour_loc);
  grid.at(random_neighbour_loc).state = State::Active;

  if (is_directly_left_of(loc, random_neighbour_loc)) {
    assert(grid.at(random_neighbour_loc).wall_right == true);
    grid.at(random_neighbour_loc).wall_right = false;
    return;
  }

  if (is_directly_above(loc, random_neighbour_loc)) {
    assert(grid.at(random_neighbour_loc).wall_down == true);
    grid.at(random_neighbour_loc).wall_down = false;
    return;
  }

  if (is_directly_below(loc, random_neighbour_loc)) {
    assert(grid.at(loc).wall_down == true);
    grid.at(loc).wall_down = false;
    return;
  }

  if (is_directly_right_of(loc, random_neighbour_loc)) {
    assert(grid.at(loc).wall_right == true);
    grid.at(loc).wall_right = false;
    return;
  }

  assert(false);
  return;
}

}  // namespace

outcome::result<SquareRectangularMazeData> GenerateMaze(int num_cols,
                                                        int num_rows) {
  util::Grid<Cell> grid =
      OUTCOME_TRYX(util::Grid<Cell>::Make(num_cols, num_rows, Cell{}));

  std::vector<Loc> active_set;
  active_set.push_back(random_location(grid));
  grid.at(active_set.back()).state = State::Active;

  while (not_finished(grid, active_set)) {
    auto next = next_loc(active_set);
    assert(next != active_set.end());

    if (has_no_new_neighbours(grid, *next)) {
      remove_location(active_set, next);
      continue;
    }

    merge_random_neighbour(grid, active_set, *next);
  }

  return grid_to_maze(grid);
}

outcome::result<std::vector<SquareRectangularMazeData>> GenerateMazeWithSteps(
    int num_cols, int num_rows) {
  util::Grid<Cell> grid =
      OUTCOME_TRYX(util::Grid<Cell>::Make(num_cols, num_rows, Cell{}));

  std::vector<Loc> active_set;
  active_set.push_back(random_location(grid));
  grid.at(active_set.back()).state = State::Active;

  std::vector<SquareRectangularMazeData> sequence;
  sequence.reserve(static_cast<size_t>(grid.num_cols() * grid.num_rows()));

  sequence.push_back(OUTCOME_TRYX(grid_to_maze(grid)));

  while (not_finished(grid, active_set)) {
    auto next = next_loc(active_set);
    assert(next != active_set.end());

    if (has_no_new_neighbours(grid, *next)) {
      remove_location(active_set, next);
      continue;
    }

    merge_random_neighbour(grid, active_set, *next);
    sequence.push_back(OUTCOME_TRYX(grid_to_maze(grid)));
  }

  return sequence;
}
}  // namespace maze_walker
