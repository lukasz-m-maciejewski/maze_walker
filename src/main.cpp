#include <docopt/docopt.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <algorithm>
#include <bitset>
#include <filesystem>
#include <functional>
#include <optional>
#include <outcome.hpp>
#include <system_error>
#include <tuple>
#include <vector>

#include "solarized.hpp"
#include "square_rectangular_maze.pb.h"

namespace outcome = OUTCOME_V2_NAMESPACE;
namespace maze_walker {

using CellWalls = SquareRectangularMazeData::CellWalls;

class SquareRectangularMaze {
  SquareRectangularMazeData data_;

 public:
  static outcome::result<SquareRectangularMaze> CreateRandomMaze(int num_rows,
                                                                 int num_cols);

  int num_rows() const { return data_.num_rows(); }
  int num_cols() const { return data_.num_cols(); }

  class ValidPosition {
    int row_;
    int col_;

    ValidPosition(int row, int col) : row_{row}, col_{col} {}
    friend class SquareRectangularMaze;

   public:
    int row() const { return row_; }
    int col() const { return col_; }
  };

  outcome::result<ValidPosition> MakePosition(int row, int col) {
    if (row < 0 || row >= num_rows()) {
      return outcome::failure(std::errc::invalid_argument);
    }

    if (col < 0 || col >= num_cols()) {
      return outcome::failure(std::errc::invalid_argument);
    }

    return outcome::success(ValidPosition{row, col});
  }

  bool has_wall_north(const ValidPosition& pos) const {
    return data_.walls(pos2idx(pos)).north();
  }

  bool has_wall_east(const ValidPosition& pos) const {
    return data_.walls(pos2idx(pos)).east();
  }

  bool has_wall_south(const ValidPosition& pos) const {
    return data_.walls(pos2idx(pos)).south();
  }

  bool has_wall_west(const ValidPosition& pos) const {
    return data_.walls(pos2idx(pos)).west();
  }

  std::bitset<4> walls_set(const ValidPosition& pos) const {
    std::bitset<4> walls;
    walls[0] = has_wall_north(pos);
    walls[1] = has_wall_east(pos);
    walls[2] = has_wall_south(pos);
    walls[3] = has_wall_west(pos);
    return walls;
  }

 private:
  int pos2idx(const ValidPosition& pos) const {
    return num_cols() * pos.row() + pos.col();
  }
};

sf::FloatRect ComputeAspectPreservingViewport(const sf::Vector2u& screen_size) {
  if (screen_size.x >= screen_size.y) {
    const float dim_ratio_inv =
        static_cast<float>(screen_size.y) / static_cast<float>(screen_size.x);
    const float left_margin = (1.0f - dim_ratio_inv) * 0.5f;
    return sf::FloatRect{left_margin, 0.0f, dim_ratio_inv, 1.0f};
  }

  const float dim_ratio_inv =
      static_cast<float>(screen_size.x) / static_cast<float>(screen_size.y);
  const float top_margin = (1.0f - dim_ratio_inv) * 0.5f;
  return sf::FloatRect{0.0f, top_margin, 1.0f, dim_ratio_inv};
}
// clang-format off
outcome::result<void> Main(
  [[maybe_unused]] const std::vector<std::string>& args) {
  // clang-format on
  // Use the default logger (stdout, multi-threaded, colored)
  spdlog::info("Hello, {}!", "World");

  sf::RenderWindow window(sf::VideoMode(1024, 768), "MazeWalker");
  window.setFramerateLimit(60);
  ImGui::SFML::Init(window);

  constexpr auto scale_factor = 2.0;
  ImGui::GetStyle().ScaleAllSizes(scale_factor);
  ImGui::GetIO().FontGlobalScale = scale_factor;

  sf::FloatRect viewport_debug{};

  bool show_overlay = false;

  sf::Clock deltaClock;
  while (window.isOpen()) {
    sf::Event event{};
    while (window.pollEvent(event)) {
      if (show_overlay) ImGui::SFML::ProcessEvent(event);

      if (event.type == sf::Event::Closed) {
        window.close();
      }

      // catch the resize events
      if (event.type == sf::Event::Resized) {
        // auto viewport =
        //     tictactoe::ComputeAspectPreservingViewport(window.getSize());
        // viewport_debug = viewport;
        // auto view = g.get_view();
        // view.setViewport(viewport);
        // window.setView(view);
      }

      if (event.type == sf::Event::MouseButtonReleased) {
        [[maybe_unused]] const sf::Vector2f mouse_pos_world =
            window.mapPixelToCoords(sf::Mouse::getPosition(window));

        spdlog::info("click at ({}, {})", mouse_pos_world.x, mouse_pos_world.y);
      }
    }

    if (show_overlay) {
      const auto window_size = window.getSize();
      const auto window_size_text =
          fmt::format("window size: {}x{}", window_size.x, window_size.y);
      const auto viewport_text = fmt::format(
          "viewport: {} {} {} {}", viewport_debug.left, viewport_debug.top,
          viewport_debug.height, viewport_debug.width);

      ImGui::SFML::Update(window, deltaClock.restart());
      ImGui::Begin("Debug info");
      ImGui::TextUnformatted(window_size_text.c_str());
      ImGui::TextUnformatted(viewport_text.c_str());
      ImGui::End();
    }

    window.clear(sf::Color::Black);

    if (show_overlay) ImGui::SFML::Render(window);

    window.display();
  }

  ImGui::SFML::Shutdown();

  return outcome::success();
}
}  // namespace maze_walker

int main(int argc, const char** argv) {
  std::vector<std::string> args = {argv, argv + argc};
  spdlog::info("args[0]:{}", args[0]);
  auto result = maze_walker::Main(args);
  return result ? 0 : 1;
}
