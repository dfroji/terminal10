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
  generate_letters(100);
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
  current_letter_ = current_letter_->next;
  current_letter_->status = active;
}

void Game::go_to_prev_letter() {
  current_letter_->status = inactive;
  current_letter_ = current_letter_->prev;
  current_letter_->status = active;
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
