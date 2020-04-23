#pragma once

#include <outcome.hpp>
#include <system_error>
#include <vector>

namespace outcome = OUTCOME_V2_NAMESPACE;

namespace util {
template <typename T>
class Grid {
  int num_rows_;
  int num_cols_;
  std::vector<T> fields_;

  using size_type = typename std::vector<T>::size_type;

  Grid(int num_rows, int num_cols) : Grid{num_rows, num_cols, T{}} {}

  Grid(int num_rows, int num_cols, T&& default_value)
      : num_rows_{num_rows},
        num_cols_{num_cols},
        fields_(static_cast<size_type>(num_cols_ * num_rows_),
                std::forward<T>(default_value)) {}

 public:
  static outcome::result<Grid<T>> Make(int num_rows, int num_cols,
                                       T&& default_value) {
    if (num_cols <= 0 || num_rows <= 0) {
      return outcome::failure(std::errc::invalid_argument);
    }

    return outcome::success(
        Grid<T>{num_rows, num_cols, std::forward<T>(default_value)});
  }

  static outcome::result<Grid<T>> Make(int num_rows, int num_cols) {
    if (num_cols <= 0 || num_rows <= 0) {
      return outcome::failure(std::errc::invalid_argument);
    }

    return outcome::success(Grid<T>{num_rows, num_cols});
  }

  int num_rows() const { return num_rows_; }
  int num_cols() const { return num_cols_; }

  class Location {
    int row_;
    int col_;

    Location(int row, int col) : row_{row}, col_{col} {}

    friend class Grid;

   public:
    int row() const { return row_; }
    int col() const { return col_; }
  };

  outcome::result<Location> MakeLocation(int row, int col) const {
    if (col < 0 || col >= num_cols() || row < 0 || row >= num_rows()) {
      return outcome::failure(std::errc::invalid_argument);
    }
    return outcome::success(Location{row, col});
  }

  T& at(const Location& loc) { return fields_[loc2idx(loc)]; }

  const T& at(const Location& loc) const { return fields_[loc2idx(loc)]; }

 private:
  size_type loc2idx(const Location& loc) const {
    return static_cast<size_type>(loc.row() * num_cols() + loc.col());
  }
};

}  // namespace util
