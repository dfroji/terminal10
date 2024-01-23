#include "Game.h"

#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cmath>

Game::Game(std::string word_file, int length) {

  length_ = length;

  // Read the given word file for words
  std::ifstream File(word_file);
  std::string line;
  while (std::getline(File, line)) {
    wordlist_.emplace_back(line);
  }

  // Generate the initial letters
  generate_letters();
}

Game::~Game() {

  // Free memory from the Letters
  Letter* current = first_letter_;
  while (current != nullptr) {
    Letter* to_be_deleted = current;
    current = current->next;
    delete to_be_deleted;
  }
}

void Game::generate_letters() {

  mistakes_by_word_.resize(std::size(mistakes_by_word_) + GENERATED_WORDS);

  // Use the current time as a seed for the random number generator
  std::srand(std::time(nullptr));

  // Generate the letters of n words.
  for (int i = 0; i < GENERATED_WORDS; i++) {
    std::string word = wordlist_[std::rand() % wordlist_.size()];

    for (char c : word) {

      // If no Letter exists, first_letter_ becomes the new Letter.
      // Otherwise it's always last_letter_->next.
      if (first_letter_ == nullptr) {
        first_letter_ = new Letter({
            c, Status(active), 0, "", nullptr, nullptr
            });
        current_letter_ = first_letter_;
        last_letter_ = first_letter_;

      } else {
        last_letter_->next = new Letter({
            c, Status(inactive), last_letter_->index + 1, 
            "", nullptr, last_letter_
            });
        last_letter_ = last_letter_->next;
      }
    }

    // Add a space after the word
    last_letter_->next = new Letter({
        ' ', Status(inactive), last_letter_->index + 1, 
        "", nullptr, last_letter_
        });
    last_letter_ = last_letter_->next;
  }
}

Letter* Game::get_first_letter() {
  return first_letter_;
}

Letter* Game::get_current_letter() {
  return current_letter_;
}

void Game::go_to_next_letter() {
  if (current_letter_->character == ' ') {
    current_word_ += 1;
    mistakes_by_word_[current_word_] = 0;
  }

  // Generate new Letters if the buffer is reached
  if (current_word_ >= total_words_ - WORD_BUFFER) {generate_letters();}

  current_letter_ = current_letter_->next;
  current_letter_->status = active;

}

void Game::go_to_prev_letter() {
  current_letter_->status = inactive;
  current_letter_ = current_letter_->prev;
  current_letter_->status = active;
  
  if (current_letter_->character == ' ') {
    mistakes_by_word_[current_word_] = -1;
    current_word_ -= 1;
  }
}

Letter* Game::get_nth_previous(int n) {
  Letter* l = current_letter_;
  for (int i = 0; i < n; i++) {
    l = l->prev;
  }
  return l;
}

Letter* Game::get_nth_next(int n) {
  Letter* l = current_letter_;
  for (int i = 0; i < n; i++) {
    l = l->next;
  }
  return l;
}

std::string Game::get_character(Letter* l) {
  std::string s{l->character};
  return s;
}

void Game::add_mistake() {
  mistakes_ += 1;
  total_mistakes_ += 1;
  mistakes_by_word_[current_word_] += 1;
}

void Game::remove_mistake() {
  mistakes_ -= 1;
  mistakes_by_word_[current_word_] -= 1;
}

int Game::get_mistakes() {
  return mistakes_;
}

int Game::get_total_mistakes() {
  return total_mistakes_;
}

int Game::get_wpm() {
  double correct_words = 0;
  for (int i = 0; i < current_word_; i++) {
    if (mistakes_by_word_[i] == 0) {correct_words += 1;}
  }

  return std::floor(correct_words / length_ * 60); 
}
