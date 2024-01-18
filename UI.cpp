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

  // Entries for a toggle
  std::vector<std::string> lengths = {
    "15s",
    "30s",
    "60s"
  };
  int selected_length = 0;

  auto menu = Container::Vertical({
      Container::Horizontal({
          // Start button initializes a Game and runs the game UI
          Button("Start", [&]{
              std::string length = lengths[selected_length];
              length.pop_back(); // Pop the 's' from the toggle's entries
              Game* game = new Game(WORD_FILE);
              game_view(std::stoi(length), game, screen.dimx());
              }) | center,

          // Quit button
          Button("Quit", screen.ExitLoopClosure()) | center,
          }) | hcenter,

      // Game length toggle
      Container::Horizontal({
          Toggle(&lengths, &selected_length) | borderLight
          }) | hcenter,

  });

  screen.Loop(menu);
}

void game_view(int length, Game* game, int screen_width){
  int time_left = length;

  auto screen = ScreenInteractive::Fullscreen();

  // Component containing the game UI rendering
  auto component = Container::Vertical({
      Renderer([&]{return text(std::to_string(time_left));}),
      Renderer([&]{return render_command(game, screen_width);}),
      });

  component |= CatchEvent([&](Event event) {
      Letter* letter = game->get_current_letter();
      std::string s = game->get_character(letter);
  
      // Correct input
      if (event.character() == s) {
        letter->status = correct;
        game->go_to_next_letter();
        return true;

      // Incorrect input
      } else if (event.is_character()) {
        letter->status = incorrect;
        letter->input = event.character();
        game->go_to_next_letter();
        return true;

      // Input is backspace
      } else if (event == Event::Backspace && letter->prev != nullptr) {
        game->go_to_prev_letter();
        return true;

      // Update screen_width with every caught event
      } else {
        screen_width = screen.dimx();

      }

      return false;
      });

  // Thread for a custom event to update the game UI every second.
  // Exit the loop once time runs out.
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

// Handles the rendering of game UI letters
Element render_command(Game* game, int& screen_width) {
    
  Elements line;
  int n = screen_width / 2;
  int letter_index = game->get_current_letter()->index;

  // Buffer spaces to keep the current letter in the middle of the terminal
  if (letter_index < n) {
    for (int i = 0; i < n - letter_index; i++) {
      line.push_back(text(" "));
    }
  }
  
  // n previous letters in front of the current letter
  for (int i = 0; i < n; i++) {
    if (letter_index >= n - i) {
      Letter* letter = game->get_nth_previous(n-i);
      std::string chr = game->get_character(letter);

      // Set the color of the character based on the letter's status
      Color clr;
      switch(letter->status) {
        case correct:
        clr = Color::GreenLight;
        break;

        case incorrect:
        // Render the incorrect input instead of the original character
        chr = letter->input;
        clr = Color::Red;
      }

      // Color the background if chr is an incorrectly input space.
      // Otherwise color the character itself
      Decorator dcr;
      if (chr == " " && letter->status == incorrect) {
        dcr = bgcolor(clr);

      } else {
        dcr = color(clr);

      }

      line.push_back(text(chr) | dcr);

    }
  }

  // Render the current letter along with the next n letters
  for (int i = 0; i < n; i++) {
    Letter* l = game->get_nth_next(i);

    // Set color to white if it is the current letter.
    // Otherwise keep the color gray.
    Color c = Color::GrayDark;
    switch(l->status) {
      case active:
      c = Color::White;
      break;
    }

    line.push_back(text(game->get_character(l)) | color(c));
  }

  // Return characters in a horizontal box for rendering
  return hbox(line);
}
