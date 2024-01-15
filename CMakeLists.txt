
cmake_minimum_required(VERSION 3.1)

project(terminal10E
  LANGUAGES CXX
  )

include(FetchContent)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v5.0.0
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_MakeAvailable(ftxui)

add_executable(terminal10 main.cpp UI.cpp Game.cpp)
target_include_directories(terminal10 PRIVATE src)

target_link_libraries(terminal10 
  PRIVATE ftxui::component
  PRIVATE ftxui::dom
  PRIVATE ftxui::screen
  )
