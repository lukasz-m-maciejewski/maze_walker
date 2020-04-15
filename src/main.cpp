#include <docopt/docopt.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <optional>
#include <outcome.hpp>
#include <system_error>
#include <tuple>
#include <vector>

#include "solarized.hpp"

namespace outcome = OUTCOME_V2_NAMESPACE;
namespace game {

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

  sf::RenderWindow window(sf::VideoMode(1024, 768), "ImGui + SFML = <3");
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
}  // namespace game

int main(int argc, const char** argv) {
  std::vector<std::string> args = {argv, argv + argc};
  spdlog::info("args[0]:{}", args[0]);
  auto result = game::Main(args);
  return result ? 0 : 1;
}
