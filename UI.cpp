#include "UI.h"

#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>

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
              game_view(std::stoi(length), game, screen.dimx());
              }) | center,
          Button("Quit", screen.ExitLoopClosure()) | center,
          }) | hcenter,
      Container::Horizontal({
          Toggle(&lengths, &selected_length) | borderLight
          }) | hcenter,
  });

  screen.Loop(menu);
}

void game_view(int length, Game* game, int screen_width){
  int time_left = length;

  auto screen = ScreenInteractive::Fullscreen();

  auto component = Container::Vertical({
      Renderer([&]{return text(std::to_string(time_left));}),
      Renderer([&]{
          Elements line;
          int n = screen_width / 2;
          int letter_index = game->get_current_letter()->index;

          if (letter_index < n) {
            for (int i = 0; i < n - letter_index; i++) {
              line.push_back(text(" "));
            }
          }

          for (int i = 0; i < n; i++) {
            if (letter_index >= n - i) {
              Letter* l = game->get_nth_previous(n-i);
              std::string str = game->get_character(l);
              Color c = Color::White;
              switch(l->status) {
                case correct:
                c = Color::GreenLight;
                break;
                case incorrect:
                str = l->input;
                c = Color::Red;
              }
              Decorator d;
              if (str == " " && l->status == incorrect) {
                d = bgcolor(c);
              } else {
                d = color(c);
              }
              line.push_back(text(str) | d);

            }
          }

          for (int i = 0; i < n; i++) {
            Letter* l = game->get_nth_next(i);
            Color c = Color::GrayDark;
            switch(l->status) {
              case active:
              c = Color::White;
              break;
            }
            line.push_back(text(game->get_character(l)) | color(c));
          }
          return hbox(line);
          }),
      });

  component |= CatchEvent([&](Event event) {
      Letter* letter = game->get_current_letter();
      std::string s = game->get_character(letter);
      if (event.character() == s) {
        letter->status = correct;
        game->go_to_next_letter();
        return true;

      } else if (event.is_character()) {
        letter->status = incorrect;
        letter->input = event.character();
        game->go_to_next_letter();
        return true;

      } else if (event == Event::Backspace && letter->prev != nullptr) {
        game->go_to_prev_letter();
        return true;

      } else {
        screen_width = screen.dimx();
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
      screen.ExitLoopClosure()();
    });

  screen.Loop(component);
  refresh.join();
  delete game;
}
