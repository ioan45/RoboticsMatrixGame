#pragma once

#include <Arduino.h>

namespace Constants {

  inline constexpr uint8_t buzzerPin = 9;
  inline constexpr uint8_t joyXPin = A0;
  inline constexpr uint8_t joyYPin = A1;
  inline constexpr uint8_t joySWPin = 2;
  inline constexpr uint8_t matrixDINPin = 13;
  inline constexpr uint8_t matrixClockPin = 12;
  inline constexpr uint8_t matrixLoadPin = 6;
  inline constexpr uint8_t lcdRSPin = 1;
  inline constexpr uint8_t lcdEnablePin = 8;
  inline constexpr uint8_t lcdD4Pin = 7;
  inline constexpr uint8_t lcdD5Pin = 11;
  inline constexpr uint8_t lcdD6Pin = 3;
  inline constexpr uint8_t lcdD7Pin = 4;
  inline constexpr uint8_t lcdContrastPin = 5;
  inline constexpr uint8_t lcdBrightnessPin = 10;
  inline constexpr uint8_t freeAnalogPin = A3;

  inline constexpr uint8_t noLcdLines = 2;
  inline constexpr uint8_t noLcdColumns = 16;
  inline constexpr uint8_t matrixSize = 8;

  inline constexpr uint8_t usernameSize = 15;
  inline constexpr uint8_t noHighscores = 5;
  inline constexpr uint8_t maxScoreDigits = 9;
  
  inline constexpr uint8_t usernameAddr = 0;
  inline constexpr uint8_t lcdContrastAddr = usernameSize;
  inline constexpr uint8_t lcdBrightAddr = lcdContrastAddr + 1;
  inline constexpr uint8_t matrixBrightAddr = lcdBrightAddr + 1;
  inline constexpr uint8_t soundsOnAddr = matrixBrightAddr + 1;
  inline constexpr uint8_t difficultyAddr = soundsOnAddr + 1;
  inline constexpr uint8_t noSavedScoresAddr = difficultyAddr + 1;
  inline constexpr uint16_t highscoresAddr = noSavedScoresAddr + 700;
  
  enum class Difficulty : uint8_t { EASY, MEDIUM, HARD, };
}
