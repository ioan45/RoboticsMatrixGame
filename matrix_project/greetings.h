#pragma once

#include "scene.h"

class Greetings : Scene {
  public:
    void start();
    void update();
  
  private:
    void cursorBlinking();
    void resetBlinking();
    void charWriting();

  private:
    uint8_t textPtr;
    uint16_t charWriteDelay;
    uint16_t blinkStateDelay;
    uint32_t lastWriteTime;
    uint32_t lastBlinkTime;
    bool cursorShown;
    bool changeScene;
};