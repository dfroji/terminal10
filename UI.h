#pragma once

#include "Game.h"

#include "ftxui/dom/elements.hpp"

void menu();
void game_view(int length, Game* game, int screen_width);
ftxui::Element render_command(Game* game, int& screen_width);
