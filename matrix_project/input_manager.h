#pragma once

#include <stdint.h>

namespace InputManager {
  
  enum class JoyDirection : uint8_t {NEUTRAL, LEFT, RIGHT, UP, DOWN};

  extern JoyDirection currentJoyDir;
  extern bool newJoyDir;
  extern bool buttonWasClicked;

  void init();
  void getUserInput();
  void joyButtonISR();
}
