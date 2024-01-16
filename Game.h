#pragma once

#include <string>
#include <vector>

enum Status {inactive, active, correct, incorrect};

struct Letter {
  char character;
  Status status;
  Letter* next;
  Letter* prev;
};

class Game {
  public:
    Game(std::string word_file);
    ~Game();

    // Get n words from words_ and generate Letters out of them
    void generate_letters(int n);

    // Return first_letter_
    Letter* get_first_letter();
    
    // Return current_letter_
    Letter* get_current_letter();
    
    // Change current_letter_ to its ->next
    void go_to_next_letter();

  private:
    std::vector<std::string> words_;

    Letter* first_letter_ = nullptr;
    Letter* current_letter_ = nullptr;
    Letter* last_letter_ = nullptr;
};
