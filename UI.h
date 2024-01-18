#pragma once

#include "Game.h"

#include "ftxui/dom/elements.hpp"

// Handle the menu UI
void menu();

// Handle the gmae UI
void game_view(int length, Game* game, int screen_width);

// Function for rendering letters in the game UI
ftxui::Element render_command(Game* game, int& screen_width);
