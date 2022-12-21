#include <Arduino.h>
#include "constants.h"
#include "input_manager.h"

namespace InputManager {

  JoyDirection currentJoyDir = JoyDirection::NEUTRAL;
  bool newJoyDir = false; 
  bool buttonWasClicked = false;

  constexpr uint8_t debounceDelay = 50;
  constexpr uint16_t joyLowerDirThreshold = 225;
  constexpr uint16_t joyHigherDirThreshold = 825;
  constexpr uint16_t joyMinNeutralValue = 425;   // The joystick potentiometer reading for a given position can also return close values.  
  constexpr uint16_t joyMaxNeutralValue = 625;   // Solves the problem when the joystick is at the exact threshold between neutral and direction.

  uint8_t currentJoySWState = HIGH;
  volatile uint32_t lastSWChangeTime = 0; 

  void init() {
    pinMode(Constants::joyXPin, INPUT);
    pinMode(Constants::joyYPin, INPUT);
    pinMode(Constants::joySWPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(Constants::joySWPin), joyButtonISR, CHANGE);
  }

  void getJoyDirection() {
    // Taking the first read direction
    JoyDirection prevDir = currentJoyDir;
    uint16_t xReading = analogRead(Constants::joyXPin);
    uint16_t yReading = analogRead(Constants::joyYPin);
    if (xReading < joyLowerDirThreshold)
      currentJoyDir = JoyDirection::LEFT;
    else if (xReading > joyHigherDirThreshold)
      currentJoyDir = JoyDirection::RIGHT;
    else if (yReading < joyLowerDirThreshold)
      currentJoyDir = JoyDirection::UP;
    else if (yReading > joyHigherDirThreshold)
      currentJoyDir = JoyDirection::DOWN;
    else if (joyMinNeutralValue < xReading && xReading < joyMaxNeutralValue && 
            joyMinNeutralValue < yReading && yReading < joyMaxNeutralValue)
      currentJoyDir = JoyDirection::NEUTRAL;
    
    // It signals when the joystick changes direction.
    // If this becomes true, it will stay like that until the next loop iteration.
    newJoyDir = (prevDir == JoyDirection::NEUTRAL && currentJoyDir != JoyDirection::NEUTRAL);
  }

  void getButtonState() {
    buttonWasClicked = false;

    // Debouncing 
    uint8_t currentSWReading = digitalRead(Constants::joySWPin);
    if (millis() - lastSWChangeTime >= debounceDelay) {
      // The current signal (LOW or HIGH) is persistent enough to take it as actual state of the button.
      if (currentSWReading != currentJoySWState) {
        // Button state has changed. Update with the new state.
        currentJoySWState = currentSWReading;
        if (currentJoySWState == LOW)
          buttonWasClicked = true;
      }
    }
  }

  void getUserInput() {
    getJoyDirection();
    getButtonState();
  }

  void joyButtonISR() {
    lastSWChangeTime = millis();
  }
}