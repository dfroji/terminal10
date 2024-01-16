#include "Game.h"

#include <fstream>
#include <cstdlib>
#include <ctime>

Game::Game(std::string word_file) {
  std::ifstream File(word_file);
  std::string line;
  while (std::getline(File, line)) {
    words_.emplace_back(line);
  }
  generate_letters(10);
}

Game::~Game() {
  Letter* current = first_letter_;
  while (current != nullptr) {
    Letter* to_be_deleted = current;
    current = current->next;
    delete to_be_deleted;
  }
}

void Game::generate_letters(int n) {
  std::srand(std::time(nullptr));
  for (int i = 0; i < n; i++) {
    std::string word = words_[std::rand() % words_.size()];

    for (char c : word) {
      if (first_letter_ == nullptr) {
        first_letter_ = new Letter({
            c, Status(active), nullptr, nullptr
            });
        current_letter_ = first_letter_;
        last_letter_ = first_letter_;

      } else {
        last_letter_->next = new Letter({
            c, Status(inactive), nullptr, last_letter_
            });
        last_letter_ = last_letter_->next;
      }
    }

    last_letter_->next = new Letter({
        ' ', Status(inactive), nullptr, last_letter_
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
  current_letter_ = current_letter_->next;
}
