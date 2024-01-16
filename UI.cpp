#include "UI.h"

#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/terminal.hpp"
#include "ftxui/component/event.hpp"

using namespace ftxui;

const std::string WORD_FILE = "../english-common.txt";

void menu(){

  auto screen = ScreenInteractive::Fullscreen();

  std::vector<std::string> lengths = {
    "15s",
    "30s",
    "60s"
  };
  int selected_length = 0;

  auto menu = Container::Vertical({
      Container::Horizontal({
          Button("Start", [&]{
              std::string length = lengths[selected_length];
              length.pop_back();
              Game* game = new Game(WORD_FILE);
              game_view(std::stoi(length), game);
              }) | center,
          Button("Quit", screen.ExitLoopClosure()) | center,
          }) | hcenter,
      Container::Horizontal({
          Toggle(&lengths, &selected_length) | borderLight
          }) | hcenter,
  });

  screen.Loop(menu);
}

void game_view(int length, Game* game){
  int time_left = length;

  auto screen = ScreenInteractive::Fullscreen();
  auto component = Container::Vertical({
      Renderer([&]{return text(std::to_string(time_left));}),
      Renderer([&]{
          std::string s{game->get_current_letter()->character};
          return text(s);
          }),
      });

  component |= CatchEvent([&](Event event) {
      std::string s{game->get_current_letter()->character};
      if (event.character() == s) {
        game->go_to_next_letter();

        return true;
      }
      return false;
      });

  std::thread refresh([&] {
      while (time_left > 1) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
        screen.Post([&]{time_left--;});
        screen.Post(Event::Custom);
      }
      delete game;
      screen.ExitLoopClosure()();
    });

  screen.Loop(component);
  refresh.join();
}
