#pragma once

#include "scene.h"

class GameOver : Scene {
  private:
    using GameOverAction = void (GameOver::*)();
    using PtrToNameArray = char(*)[Constants::usernameSize];  // Allows EEPROM get() / put() to be used with a heap array.

  public:
    void start();
    void update();
    GameOver();

  private:
    void gameOverMsg();
    void infoScreen();
    void createHighscoreMsg();
    void findAchievedPlace();
    void loadHighscores();
    void saveScore();
    void cleanup();

  private:
    GameOverAction activeAction;
    char* congratsMsg;
    char* scoreMsg;
    char* highscoreMsg;
    char* msgToScroll;
    uint8_t congratsMsgLen;
    uint8_t scoreMsgLen;
    uint8_t highscoreMsgLen; 
    uint8_t msgToScrollLen;
    uint8_t msgToScrollLine;
    uint8_t scrollingTextIndex;
    uint8_t topPlaceAchieved;
    uint8_t noSavedScores;
    uint32_t lastTextScrollTime;
    uint32_t* scoreValues;
    char** scoreNames;
    const uint16_t scrollingTextSpeed;
    const uint16_t clickToneFreq;
    const uint16_t toneDuration;
};