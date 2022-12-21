#include <stdlib.h>
#include <stdio.h>
#include <EEPROM.h>
#include "constants.h"
#include "input_manager.h"
#include "renderer.h"
#include "game.h"
#include "menu.h"
#include "game_over.h"


GameOver::GameOver() 
  : scrollingTextSpeed(500),
    clickToneFreq(750),
    toneDuration(100)
{
}

void GameOver::start() {
  scoreValues = nullptr;
  scoreNames = nullptr;
  msgToScroll = nullptr;
  noSavedScores = 0;
  msgToScrollLen = 0;
  scrollingTextIndex = 0; 
  lastTextScrollTime = millis();

  congratsMsg = new char[Constants::maxScoreDigits + 36];
  sprintf(congratsMsg, "Congrats on surviving for %lus!", Game::gameDuration);
  congratsMsgLen = strlen(congratsMsg);
  scoreMsg = new char[Constants::maxScoreDigits + 13];
  sprintf(scoreMsg, "Your score: %lu", (uint32_t)Game::score);
  scoreMsgLen = strlen(scoreMsg);
  loadHighscores();
  findAchievedPlace();
  createHighscoreMsg();

  Renderer::clearMatrix();
  Renderer::lcdController.clear();
  Renderer::lcdController.setCursor(3, 0);
  Renderer::lcdController.print("Game Over!");
  Renderer::lcdController.setCursor(0, 1);
  Renderer::lcdController.print(congratsMsg);
  activeAction = &GameOver::gameOverMsg;
}

void GameOver::update() {
  if (activeAction)
    (this->*activeAction)();
}

void GameOver::gameOverMsg() {
  if (InputManager::buttonWasClicked) {
    Renderer::lcdController.clear();
    Renderer::lcdController.print(scoreMsg);
    Renderer::lcdController.setCursor(0, 1);
    Renderer::lcdController.print(highscoreMsg);
    Renderer::playSound(clickToneFreq, toneDuration);
    msgToScroll = scoreMsg;
    msgToScrollLen = scoreMsgLen;
    msgToScrollLine = 0;
    scrollingTextIndex = 0;
    lastTextScrollTime = millis();
    activeAction = &GameOver::infoScreen;
  }

  if (millis() - lastTextScrollTime >= scrollingTextSpeed) {
    ++scrollingTextIndex;
    if (scrollingTextIndex == congratsMsgLen - Constants::noLcdColumns + 1)
      scrollingTextIndex = 0;
    Renderer::lcdController.setCursor(0, 1);
    Renderer::lcdController.print(congratsMsg + scrollingTextIndex);
    lastTextScrollTime = millis();
  }
}

void GameOver::infoScreen() {
  if (InputManager::buttonWasClicked) {
    saveScore();
    cleanup();
    Renderer::playSound(clickToneFreq, toneDuration);
    Scene::playNewScene<Menu>();    
    return;
  }

  if (InputManager::newJoyDir) {
    if (InputManager::currentJoyDir == InputManager::JoyDirection::DOWN && msgToScrollLine != 1) {
      Renderer::lcdController.setCursor(0, msgToScrollLine);
      Renderer::lcdController.print(msgToScroll);
      msgToScroll = highscoreMsg;
      msgToScrollLen = highscoreMsgLen;
      msgToScrollLine = 1;
      scrollingTextIndex = 0;
      lastTextScrollTime = millis();
    }
    else if (InputManager::currentJoyDir == InputManager::JoyDirection::UP && msgToScrollLine != 0) {
      Renderer::lcdController.setCursor(0, msgToScrollLine);
      Renderer::lcdController.print(msgToScroll);
      msgToScroll = scoreMsg;
      msgToScrollLen = scoreMsgLen;
      msgToScrollLine = 0;
      scrollingTextIndex = 0;
      lastTextScrollTime = millis();
    }
  }

  if (msgToScrollLen > Constants::noLcdColumns && millis() - lastTextScrollTime >= scrollingTextSpeed) {
    ++scrollingTextIndex;
    if (scrollingTextIndex == msgToScrollLen - Constants::noLcdColumns + 1)
      scrollingTextIndex = 0;
    Renderer::lcdController.setCursor(0, msgToScrollLine);
    Renderer::lcdController.print(msgToScroll + scrollingTextIndex);
    lastTextScrollTime = millis();
  }
} 

void GameOver::createHighscoreMsg() {
  constexpr uint8_t maxHighscoreMsgLen = 42;
  highscoreMsg = new char[maxHighscoreMsgLen];
  const char* place = nullptr; 
  switch (topPlaceAchieved) {
    case 1:
      place = "1st";
      break;
    case 2:
      place = "2nd";
      break;
    case 3:
      place = "3rd";
      break;
    case 4:
      place = "4th";
      break;
    case 5:
      place = "5th";
      break;
  }
  if (place)
    sprintf(highscoreMsg, "You got %s place in top highscores!", place);
  else
    strcpy(highscoreMsg, "Your score is too low for top highscores");    
  highscoreMsgLen = strlen(highscoreMsg);
}

void GameOver::findAchievedPlace() {
  topPlaceAchieved = 0;
  for (uint8_t i = 0; i < noSavedScores; ++i)
    if (Game::score > scoreValues[i]) {
      topPlaceAchieved = i + 1;
      break;
    }
  if (topPlaceAchieved == 0 && noSavedScores < Constants::noHighscores)
    topPlaceAchieved = noSavedScores + 1;
}

void GameOver::loadHighscores() {
  EEPROM.get(Constants::noSavedScoresAddr, noSavedScores);
  if (noSavedScores == 0)
    return;
  scoreValues = new uint32_t[noSavedScores];
  scoreNames = new char*[noSavedScores];
  for (uint8_t i = 0; i < noSavedScores; ++i) {
    uint16_t scoreNameAddr = Constants::highscoresAddr + i * Constants::usernameSize + i * sizeof(uint32_t);
    scoreNames[i] = new char[Constants::usernameSize];
    EEPROM.get(scoreNameAddr, *(PtrToNameArray)scoreNames[i]);
    EEPROM.get(scoreNameAddr + Constants::usernameSize, scoreValues[i]);
  }
}

void GameOver::saveScore() {
  if (topPlaceAchieved > 0) {
    if (topPlaceAchieved < noSavedScores)  // don't shift if the place achieved is the last because the current last will be erased
      for (uint8_t i = noSavedScores; i >= topPlaceAchieved; --i) {
        uint16_t scoreNameAddr = Constants::highscoresAddr + i * Constants::usernameSize + i * sizeof(uint32_t);
        EEPROM.put(scoreNameAddr, *(PtrToNameArray)scoreNames[i - 1]);
        EEPROM.put(scoreNameAddr + Constants::usernameSize, scoreValues[i - 1]);
      }
    if (noSavedScores < Constants::noHighscores)
      EEPROM.put(Constants::noSavedScoresAddr, uint8_t(noSavedScores + 1));
    char username[Constants::usernameSize];
    EEPROM.get(Constants::usernameAddr, username);
    uint16_t newScoreNameAddr = Constants::highscoresAddr + (topPlaceAchieved - 1) * Constants::usernameSize + (topPlaceAchieved - 1) * sizeof(uint32_t);
    EEPROM.put(newScoreNameAddr, username);
    EEPROM.put(newScoreNameAddr + Constants::usernameSize, (uint32_t)Game::score); 
  }
}

void GameOver::cleanup() {
  delete[] congratsMsg;
  delete[] scoreMsg;
  delete[] highscoreMsg;
  if (scoreValues)
    delete[] scoreValues;
  if (scoreNames) {
    for (uint8_t i = 0; i < noSavedScores; ++i)
      delete[] scoreNames[i];
    delete[] scoreNames;
  }
}
