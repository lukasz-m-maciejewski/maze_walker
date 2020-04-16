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
namespace fs = std::filesystem;

namespace maze_walker {

struct Configuration {
  fs::path asset_dir;
};

outcome::result<fs::path> MakeAssetDir(const fs::path& start_dir,
                                       const fs::path& work_dir) {
  fs::path assets = start_dir.is_absolute() ? start_dir : work_dir / start_dir;

  assets.remove_filename();
  assets /= "../assets";

  return assets;
}

outcome::result<Configuration> MakeConfiguration(
    const std::vector<std::string>& commandline_args,
    const fs::path& work_dir) {
  Configuration config;
  config.asset_dir =
      OUTCOME_TRYX(MakeAssetDir(fs::path{commandline_args[0]}, work_dir));
  return config;
}

class SquareRectangularMaze {
  SquareRectangularMazeData data_;

 public:
  static outcome::result<SquareRectangularMaze> Make(int num_cols,
                                                     int num_rows) {
    SquareRectangularMazeData data;
    data.set_num_cols(num_cols);
    data.set_num_rows(num_rows);

    for (int i = 0; i < num_cols * num_rows; ++i) {
      auto wall = data.add_walls();
      wall->set_north(false);
      wall->set_east(false);
      wall->set_south(false);
      wall->set_west(false);
    }

    SquareRectangularMaze maze;
    maze.data_ = std::move(data);
    return outcome::success(std::move(maze));
  }

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

  outcome::result<ValidPosition> make_position(int row, int col) const {
    if (row < 0 || row >= num_rows()) {
      return outcome::failure(std::errc::invalid_argument);
    }

    if (col < 0 || col >= num_cols()) {
      return outcome::failure(std::errc::invalid_argument);
    }

    return outcome::success(ValidPosition{row, col});
  }

  bool has_wall_north(const ValidPosition& pos) const {
    if (pos.row() == 0) {
      return true;
    }
    return data_.walls(pos2idx(pos)).north();
  }

  bool has_wall_east(const ValidPosition& pos) const {
    if (pos.col() == num_cols() - 1) {
      return true;
    }
    return data_.walls(pos2idx(pos)).east();
  }

  bool has_wall_south(const ValidPosition& pos) const {
    if (pos.row() == num_rows() - 1) {
      return true;
    }
    return data_.walls(pos2idx(pos)).south();
  }

  bool has_wall_west(const ValidPosition& pos) const {
    if (pos.col() == 0) {
      return true;
    }
    return data_.walls(pos2idx(pos)).west();
  }

  std::bitset<4> walls(const ValidPosition& pos) const {
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

class TilesLibrary {
  sf::Texture texture_;
  std::vector<sf::IntRect> base_tiles_;

 public:
  static outcome::result<TilesLibrary> Make(fs::path path_to_texture) {
    TilesLibrary tl;
    const int pixel_width = 128;
    const int pixel_height = 128;
    tl.texture_.loadFromFile(path_to_texture.string());

    std::vector<std::pair<int, int>> offsets = {
        {1, 3},  // 0 - all open
        {3, 3},  // 1 - N
        {3, 2},  // 2 - E
        {8, 0},  // 3 - NE
        {2, 3},  // 4 - S
        {1, 0},  // 5 - NS
        {8, 1},  // 6 - SE
        {9, 3},  // 7 - NES
        {2, 2},  // 8 - W
        {7, 0},  // 9 - NW
        {0, 0},  // 10 - EW
        {8, 2},  // 11 - NEW
        {7, 1},  // 12 - SW
        {8, 3},  // 13 - NSW
        {9, 2},  // 14 - ESW
        {0, 2},  // 15 - NESW
    };

    using VI = sf::Vector2i;

    const auto size = VI{pixel_width, pixel_height};

    tl.base_tiles_.reserve(offsets.size());
    std::transform(
        offsets.begin(), offsets.end(), std::back_inserter(tl.base_tiles_),
        [&](const std::pair<int, int>& offset) {
          return sf::IntRect{
              VI{offset.first * pixel_width, offset.second * pixel_height},
              size};
        });

    return tl;
  }

  sf::Texture const* texture() const { return &texture_; }
  const sf::IntRect& texture_rect_for(std::bitset<4> tile_type) const {
    return base_tiles_[tile_type.to_ulong()];
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

outcome::result<void> Draw(sf::RenderTarget& target, const TilesLibrary& tl,
                           const SquareRectangularMaze& maze) {
  const auto side_size = 50.0f;
  const auto rect_size = sf::Vector2f{side_size, side_size};

  for (int row = 0; row < maze.num_rows(); ++row) {
    for (int col = 0; col < maze.num_cols(); ++col) {
      const auto maze_position = OUTCOME_TRYX(maze.make_position(row, col));
      sf::RectangleShape rect{rect_size};
      const float shift_x = side_size * static_cast<float>(col);
      const float shift_y = side_size * static_cast<float>(row);
      const auto world_position = sf::Vector2f{shift_x, shift_y};
      rect.setPosition(world_position);

      rect.setTexture(tl.texture());
      const auto walls = maze.walls(maze_position);
      rect.setTextureRect(tl.texture_rect_for(walls));

      target.draw(rect);
    }
  }

  return outcome::success();
}

outcome::result<void> Main(const std::vector<std::string>& args) {
  const Configuration config =
      OUTCOME_TRYX(MakeConfiguration(args, fs::current_path()));

  const auto road_textures_filepath = config.asset_dir / "roadTextures.png";
  const TilesLibrary tiles_library =
      OUTCOME_TRYX(TilesLibrary::Make(road_textures_filepath));

  SquareRectangularMaze maze =
      OUTCOME_TRYX(SquareRectangularMaze::Make(10, 10));

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

    OUTCOME_TRYV(Draw(window, tiles_library, maze));

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
