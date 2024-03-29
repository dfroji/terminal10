#include "UI.h"

#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#include <fstream>

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/terminal.hpp"
#include "ftxui/component/event.hpp"

using namespace ftxui;

const std::string WORD_FILE = "../wordlists/english-common.txt";

const Color BGCOLOR = Color::Black;
const Color DEFCOLOR = Color::White;
const Color CORRECTCOLOR = Color::Green;
const Color INCORRECTCOLOR = Color::Red;
const Color ACTIVECOLOR = Color::White;
const Color INACTIVECOLOR = Color::GrayDark;
const Color ACCENTCOLOR = Color::Cyan;

void game_UI(int length, Game* game, int screen_width);
Element letter_render(Game* game, int& screen_width);

MenuOption hmenu_style();
ButtonOption button_style();
std::vector<std::string> read_wordlist_file(std::string wordfile);

void UI::menu(){

  auto screen = ScreenInteractive::Fullscreen();

  // Entries for a toggle
  std::vector<std::string> lengths = {
    "15",
    "30",
    "60"
  };
  int selected_length = 0;


  // Modal component to be displayed if reading word list file fails
  bool error_shown = false;
  auto error_modal = Container::Vertical({
      Container::Horizontal({
          Button("OK", [&]{error_shown = false;}, button_style()),
          }) | center,
      Renderer([]{return text("Failed to read word list file!");})
      });

  auto menu = Container::Vertical({
      Container::Horizontal({
          // Start button initializes a Game and runs the game UI
          Button("Start", [&]{
              std::vector<std::string> wordlist = read_wordlist_file(WORD_FILE);
              if (wordlist.empty()) {
                error_shown = true;
              } else {
                std::string length = lengths[selected_length];
                Game* game = new Game(wordlist, std::stoi(length));
                game_UI(std::stoi(length), game, screen.dimx());
              }
              }, button_style()),

          // Quit button
          Button("Quit", screen.ExitLoopClosure(), button_style()),
          }) | center,

      // Game length toggle
      Container::Horizontal({
          Menu(&lengths, &selected_length, hmenu_style()),
          }) | color(DEFCOLOR),

  }) | bgcolor(BGCOLOR);

  menu |= Modal(error_modal, &error_shown);

  screen.Loop(menu);
}

void game_UI(int length, Game* game, int screen_width){

  int time_left = length;
  // Bools for refresh() thread
  bool has_started = false;
  bool is_exited = false;

  auto screen = ScreenInteractive::Fullscreen();

  // Component containing the game UI rendering
  auto main_component = Container::Vertical({
      Renderer([&]{
          return text("Time left: " + 
                    std::to_string(time_left)) | color(DEFCOLOR);
          }),
      Renderer([&]{
          return text(
              "Mistakes (total): " 
              + std::to_string(game->get_mistakes())
              + " ("
              + std::to_string(game->get_total_mistakes())
              + ")"
            ) | color(DEFCOLOR);
          }),
      Renderer([&]{return letter_render(game, screen_width);}),
      }) | bgcolor(BGCOLOR);

  // Modal component to be displayed after time has run out
  auto result = Container::Vertical({
      Button("OK", screen.ExitLoopClosure(), button_style()),
      Renderer([&]{
          return text("wpm: " + std::to_string(game->get_wpm()));
          }),
      });
  bool result_shown = false;

  main_component |= Modal(result, &result_shown);

  main_component |= CatchEvent([&](Event event) {
 
      // Blocks inputs in the main component once time has run out.
      if (result_shown) {return false;}

      Letter* letter = game->get_current_letter();
      std::string s = game->get_character(letter);
  
      // Correct input
      if (event.character() == s) {
        has_started = true;
        letter->status = correct;
        game->go_to_next_letter();
        return true;

      // Incorrect input
      } else if (event.is_character()) {
        has_started = true;
        letter->status = incorrect;
        letter->input = event.character();
        game->add_mistake();
        game->go_to_next_letter();
        return true;

      // Input is backspace
      } else if (event == Event::Backspace 
                 && letter->prev != nullptr) {
        switch (game->get_nth_previous(1)->status) {
          case incorrect:
          game->remove_mistake();

        }

        game->go_to_prev_letter();
        return true;
      
      // Input is esc
      } else if (event == Event::Escape) {
        is_exited = true;
        screen.ExitLoopClosure()();
        return true;
      }

      // Update screen_width with every caught event.
      screen_width = screen.dimx();
      
      // Trigger result screen once time runs out.
      if (time_left == 0) {
        result_shown = true;
      }

      return false;
      });

  // Thread for a custom event to update the game UI every second.
  // Exit the loop once time runs out.
  std::thread refresh([&] {
      int ticks = 0;

      // Loop until time runs out or user exits out of the game UI
      while (time_left > 0 && !is_exited) {

        // Start counting time down only after the first input
        if (!has_started) {
          continue;
        }

        // 50ms instead of 1s so the delay after 
        // exiting out of the game UI isn't so noticeable
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(50ms);
        ticks++;
        if (ticks >= 1000 / 50) {
        screen.Post([&]{time_left--;});
        screen.Post(Event::Custom);
        ticks = 0;
        }

      }
    });


  screen.Loop(main_component);
  refresh.join();
  delete game;
}

// Handles the rendering of game UI letters
Element letter_render(Game* game, int& screen_width) {
    
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
        clr = CORRECTCOLOR;
        break;

        case incorrect:
        // Render the incorrect input instead of the original character
        chr = letter->input;
        clr = INCORRECTCOLOR;
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

    // Set color to ACTIVECOLOR if it is the current letter.
    // Otherwise keep the color INCORRECTCOLOR.
    Color c = INACTIVECOLOR;
    switch(l->status) {
      case active:
      c = ACTIVECOLOR;
      break;
    }

    line.push_back(text(game->get_character(l)) | color(c));
  }

  // Return characters in a horizontal box for rendering
  return hbox(line);
}

MenuOption hmenu_style() {
  using namespace std::chrono_literals;

  auto option = MenuOption::HorizontalAnimated();
  option.underline.SetAnimation(150ms, animation::easing::BackOut);
  option.entries_option.transform = [](EntryState state) {
    Element e = text(state.label) | hcenter;
    if (state.focused)
      e = e | color(ACCENTCOLOR);
    if (state.active)
      e = e | bold;
    if (!state.focused && !state.active)
      e = e | dim;
    return e;
  };
  option.underline.color_inactive = DEFCOLOR;
  option.underline.color_active = ACCENTCOLOR;
  option.elements_prefix = []{return text(" Time »»» ");};
  option.elements_infix = []{return text(" / ");};
  option.elements_postfix = []{return text(" seconds");};

  return option;
}

ButtonOption button_style() {
  Color background = DEFCOLOR;
  Color foreground = ACCENTCOLOR;
  auto option = ButtonOption::Animated(background, foreground);

  return option;
}

std::vector<std::string> read_wordlist_file(std::string wordfile) {

  // Tries to return a list of words from given file.
  // Returns empty vector if reading the file fails.
  try {
    std::vector<std::string> wordlist;

    std::ifstream file(wordfile);
    std::string line;
    while (std::getline(file, line)) {
      wordlist.emplace_back(line);
    }
    file.close();

    return wordlist;

  }
  catch (const std::ifstream::failure& e) {
    return {};

  }  

}
