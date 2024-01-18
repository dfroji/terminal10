#pragma once

#include <string>
#include <vector>

// Status for Letters
enum Status {inactive, active, correct, incorrect};

// Type for the writing game characters
struct Letter {
  char character;
  Status status;
  int index;
  std::string input;
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

    // Change current_letter_ to its ->prev
    void go_to_prev_letter();

    // Get the nth previous letter from current_letter_
    Letter* get_nth_previous(int n);

    // Get the nth next letter from current_letter_
    Letter* get_nth_next(int n);

    // Return the given Letter's character as a string
    std::string get_character(Letter* l);

    // Add to the mistakes_ and total_mistakes_ counters
    void add_mistake();

    // Remove from the mistakes_ counter
    void remove_mistake();

    // Get mistakes_
    int get_mistakes();

    // Get total_mistakes_
    int get_total_mistakes();

  private:
    // Vector to store a word list
    std::vector<std::string> words_;

    Letter* first_letter_ = nullptr;
    Letter* current_letter_ = nullptr;
    Letter* last_letter_ = nullptr;

    int mistakes_ = 0;
    int total_mistakes_ = 0;
};