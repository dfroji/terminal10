#pragma once

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
    Game();
    ~Game();

  private:
    Letter* first_letter_;
    Letter* current_letter_;
};
