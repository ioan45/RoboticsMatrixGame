#include "renderer.h"
#include "menu.h"
#include "greetings.h"
#include "stdlib.h"


void Greetings::start() {
  Renderer::lcdController.clear();
  Renderer::lcdController.setCursor(3, 0);
  textPtr = 0;
  charWriteDelay = 2500;
  blinkStateDelay = 300;
  lastWriteTime = millis();
  lastBlinkTime = millis();
  cursorShown = true;
  changeScene = false;
}

void Greetings::update() {
  cursorBlinking();
  charWriting();
  if (changeScene) {
    Renderer::lcdController.noCursor();
    Scene::playNewScene<Menu>();
  }
}

void Greetings::cursorBlinking() {
  if (millis() - lastBlinkTime >= blinkStateDelay) {
    cursorShown = !cursorShown;
    if (cursorShown)
      Renderer::lcdController.cursor();
    else
      Renderer::lcdController.noCursor();
    lastBlinkTime = millis();
  }
}

void Greetings::resetBlinking() {
  Renderer::lcdController.cursor();
  cursorShown = true;
  lastBlinkTime = millis();
}

void Greetings::charWriting() {
  constexpr const char* text = "Welcome to\nDodge Master";
  constexpr uint8_t textLen = strlen(text);

  if (millis() - lastWriteTime >= charWriteDelay) {
    if (textPtr == textLen) {
      changeScene = true;
      return;
    }
    
    Renderer::lcdController.print(text[textPtr]);
    ++textPtr;
    lastWriteTime = millis();
    resetBlinking();

    if (textPtr == 1)
      charWriteDelay = 100;
    else if (textPtr == textLen)
      charWriteDelay = 3000;
    else if (text[textPtr] == '\n') {
      Renderer::lcdController.setCursor(2, 1);
      ++textPtr;
    }
  }
}