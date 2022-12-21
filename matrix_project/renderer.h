#pragma once

#include "LiquidCrystal.h"

namespace Renderer {
  
  extern LiquidCrystal lcdController;
  extern bool soundsOn;

  void init();
  void setMatrixBrightness(uint8_t intensityValue);
  void clearMatrix();
  void addMatrixChange(uint8_t line, uint8_t column, uint8_t newValue);
  void playSound(uint16_t freq, uint32_t duration);
  void updateMatrix(); 
}