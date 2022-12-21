#include <Arduino.h>
#include <EEPROM.h>
#include <stdlib.h>
#include "constants.h"
#include "input_manager.h"
#include "renderer.h"
#include "game.h"
#include "menu.h"


Menu::Menu()
  : toneDuration(100),
    scrollingTextSpeed(500),
    pointerBlinkDelay(400),
    scrollToneFreq(500),
    clickToneFreq(750), 
    maxStackSize(2),
    subMenusStack(nullptr)
{
}

Menu::~Menu() {
  if (currentMenuSetup) {
    delete[] currentMenuSetup->options;
    delete currentMenuSetup;
  }
  if (subMenusStack) {
    for (uint8_t i = 0; i < stackSize; ++i) {
      delete[] subMenusStack[i]->options;
      delete subMenusStack[i];
    }
    delete[] subMenusStack;
  }
}

void Menu::start() {
  subMenusStack = new SubMenuSetup*[maxStackSize]();
  stackSize = 0;

  currentMenuSetup = new SubMenuSetup();
  currentMenuSetup->loopLogic = &Menu::onMainMenu;
  currentMenuSetup->noOptions = 5;
  currentMenuSetup->options = new const char*[currentMenuSetup->noOptions] {
    "Play", 
    "How to play", 
    "Highscores", 
    "Settings", 
    "About"
  };

  currentOptionLength = strlen(currentMenuSetup->options[0]);
  currentPointedColumn = 0;
  scrollingTextIndex = 0;
  optionLocked = false;

  this->displayCurrentSetup();
}

void Menu::update() {
  MenuAction onUpdateAction = currentMenuSetup->loopLogic;
  if (onUpdateAction)
    (this->*onUpdateAction)();
  this->checkOptionScrolling();
  this->checkTextScrolling();
  this->checkOptionLocked();
}

void Menu::onMainMenu() {
  if (InputManager::buttonWasClicked) {
    Renderer::playSound(clickToneFreq, toneDuration);
    SubMenuSetup* newSetup;
    switch (currentMenuSetup->pointedOptionIndex) {
      case 0:
        Renderer::lcdController.clear();
        Scene::playNewScene<Game>();
        break;
      
      case 1:
        newSetup = new SubMenuSetup();
        newSetup->loopLogic = &Menu::onHowToPlayOption;
        newSetup->noOptions = 5;
        newSetup->options = new const char*[newSetup->noOptions] {
          "Endless runner.",
          "Dodge incoming",
          "obstacles from",
          "above.",
          "Go back to menu"
        };
        this->onEnterSubMenu(newSetup);
        break;

      case 2:
        newSetup = new SubMenuSetup();
        newSetup->loopLogic = &Menu::onHighscoresOption;
        this->loadHighscores(newSetup);        
        this->onEnterSubMenu(newSetup);
        break;
      
      case 3:
        newSetup = new SubMenuSetup();
        newSetup->loopLogic = &Menu::onSettingsOption;
        newSetup->noOptions = 7;
        newSetup->options = new const char*[newSetup->noOptions] {
          "Enter name",
          "Difficulty",
          "LCD Contrast",
          "LCD brightness",
          "Matrix brightness",
          "Sounds ON/OFF",
          "Go back to menu"
        };
        this->onEnterSubMenu(newSetup);
        break;

      case 4:
        newSetup = new SubMenuSetup();
        newSetup->loopLogic = &Menu::onAboutOption;
        newSetup->noOptions = 3;
        newSetup->options = new const char*[newSetup->noOptions] {
          "Game title: Dodge Master",
          "GitHub: https://github.com/ioan45",
          "Go back to menu"
        };
        this->onEnterSubMenu(newSetup);
        break;
    }
  }
}

void Menu::onHowToPlayOption() {
  if (InputManager::buttonWasClicked && currentMenuSetup->pointedOptionIndex == currentMenuSetup->noOptions - 1) {
    Renderer::playSound(clickToneFreq, toneDuration);
    this->onExitSubMenu();
  }
}

void Menu::onHighscoresOption() {
  if (InputManager::buttonWasClicked) {
    if (currentMenuSetup->pointedOptionIndex == currentMenuSetup->noOptions - 2) {
      Renderer::playSound(clickToneFreq, toneDuration);
      EEPROM.put(Constants::noSavedScoresAddr, (uint8_t)0);
    }
    if (currentMenuSetup->pointedOptionIndex == currentMenuSetup->noOptions - 1 ||
        currentMenuSetup->pointedOptionIndex == currentMenuSetup->noOptions - 2) {
      Renderer::playSound(clickToneFreq, toneDuration);
      for (uint8_t i = 0; i < currentMenuSetup->noOptions - 2; ++i)
        delete[] currentMenuSetup->options[i];
      this->onExitSubMenu();
    }
  }
}

void Menu::onSettingsOption() {
  if (InputManager::buttonWasClicked) {
    Renderer::playSound(clickToneFreq, toneDuration);
    if (currentMenuSetup->pointedOptionIndex == 0) {
      char* username = new char[Constants::usernameSize + 1];
      EEPROM.get(Constants::usernameAddr, *(PtrToNameArray)username);
      username[Constants::usernameSize] = '\0';
      SubMenuSetup* newSetup = new SubMenuSetup();
      newSetup->loopLogic = &Menu::onEnterNameSetting;
      newSetup->noOptions = 2;
      newSetup->options = new const char*[newSetup->noOptions] {
        username,
        "Go back to settings"
      };
      this->onEnterSubMenu(newSetup);
    }
    else if (currentMenuSetup->pointedOptionIndex == 1) {
      const char* diffText = nullptr;
      EEPROM.get(Constants::difficultyAddr, difficultyValue);
      if (difficultyValue == Constants::Difficulty::EASY)
        diffText = "EASY: Walls    ";
      else if (difficultyValue == Constants::Difficulty::MEDIUM)
        diffText = "MEDIUM: Walls, Projectiles Rain";
      else if (difficultyValue == Constants::Difficulty::HARD)
        diffText = "HARD: Walls, Projectiles Rain, Speed Up";
      SubMenuSetup* newSetup = new SubMenuSetup();
      newSetup->loopLogic = &Menu::onDifficultySetting;
      newSetup->noOptions = 2;
      newSetup->options = new const char*[newSetup->noOptions] {
        diffText,
        "Go back to settings"
      };
      this->onEnterSubMenu(newSetup);
    }
    else if (currentMenuSetup->pointedOptionIndex == 2) {
      const uint8_t unitsPerBlock = 16;
      char* blocksIndicator = this->loadBlocksIndicator(Constants::lcdContrastAddr, contrastValue, unitsPerBlock);
      SubMenuSetup* newSetup = new SubMenuSetup();
      newSetup->loopLogic = &Menu::onContrastSetting;
      newSetup->noOptions = 2;
      newSetup->options = new const char*[newSetup->noOptions] {
        blocksIndicator,
        "Go back to settings"
      };
      this->onEnterSubMenu(newSetup);
    }
    else if (currentMenuSetup->pointedOptionIndex == 3) {
      const uint8_t unitsPerBlock = 16;
      char* blocksIndicator = this->loadBlocksIndicator(Constants::lcdBrightAddr, lcdBrightValue, unitsPerBlock);
      SubMenuSetup* newSetup = new SubMenuSetup();
      newSetup->loopLogic = &Menu::onLcdBrightSetting;
      newSetup->noOptions = 2;
      newSetup->options = new const char*[newSetup->noOptions] {
        blocksIndicator,
        "Go back to settings"
      };
      this->onEnterSubMenu(newSetup);
    }
    else if (currentMenuSetup->pointedOptionIndex == 4) {
      for (uint8_t i = 0; i < Constants::matrixSize; ++i)
        for (uint8_t j = 0; j < Constants::matrixSize; ++j)
          Renderer::addMatrixChange(i, j, HIGH);
      const uint8_t unitsPerBlock = 1;
      char* blocksIndicator = this->loadBlocksIndicator(Constants::matrixBrightAddr, matrixBrightValue, unitsPerBlock);
      SubMenuSetup* newSetup = new SubMenuSetup();
      newSetup->loopLogic = &Menu::onMatrixBrightSetting;
      newSetup->noOptions = 2;
      newSetup->options = new const char*[newSetup->noOptions] {
        blocksIndicator,
        "Go back to settings"
      };
      this->onEnterSubMenu(newSetup);
    }
    else if (currentMenuSetup->pointedOptionIndex == 5) {
      EEPROM.get(Constants::soundsOnAddr, Renderer::soundsOn);
      SubMenuSetup* newSetup = new SubMenuSetup();
      newSetup->loopLogic = &Menu::onMuteSoundsSetting;
      newSetup->noOptions = 2;
      newSetup->options = new const char*[newSetup->noOptions] {
        (Renderer::soundsOn ? "ON " : "OFF"),
        "Go back to settings"
      };
      this->onEnterSubMenu(newSetup);
    }
    else if (currentMenuSetup->pointedOptionIndex == currentMenuSetup->noOptions - 1)
      this->onExitSubMenu();
  }  
}

void Menu::onEnterNameSetting() {
  if (InputManager::newJoyDir && optionLocked) {
    if (InputManager::currentJoyDir == InputManager::JoyDirection::UP) {
      char* username = (char*)currentMenuSetup->options[currentMenuSetup->pointedOptionIndex];
      if (username[currentPointedColumn - 1] == 'A')
        username[currentPointedColumn - 1] = ' ';
      else if (username[currentPointedColumn - 1] == ' ')
        username[currentPointedColumn - 1] = 'Z';
      else
        username[currentPointedColumn - 1]--;
      Renderer::lcdController.setCursor(currentPointedColumn, currentMenuSetup->pointedDisplayLine);
      Renderer::lcdController.print(username[currentPointedColumn - 1]);
      Renderer::lcdController.setCursor(currentPointedColumn, currentMenuSetup->pointedDisplayLine);
    }
    else if (InputManager::currentJoyDir == InputManager::JoyDirection::DOWN) {
      char* username = (char*)currentMenuSetup->options[currentMenuSetup->pointedOptionIndex];
      if (username[currentPointedColumn - 1] == 'Z')
        username[currentPointedColumn - 1] = ' ';
      else if (username[currentPointedColumn - 1] == ' ')
        username[currentPointedColumn - 1] = 'A';
      else
        username[currentPointedColumn - 1]++;
      Renderer::lcdController.setCursor(currentPointedColumn, currentMenuSetup->pointedDisplayLine);
      Renderer::lcdController.print(username[currentPointedColumn - 1]);
      Renderer::lcdController.setCursor(currentPointedColumn, currentMenuSetup->pointedDisplayLine);
    }
    else if (InputManager::currentJoyDir == InputManager::JoyDirection::RIGHT && currentPointedColumn < Constants::noLcdColumns - 1) {
      currentPointedColumn++;
      Renderer::lcdController.setCursor(currentPointedColumn, currentMenuSetup->pointedDisplayLine);
    }
    else if (InputManager::currentJoyDir == InputManager::JoyDirection::LEFT && currentPointedColumn > 1) {
      currentPointedColumn--;
      Renderer::lcdController.setCursor(currentPointedColumn, currentMenuSetup->pointedDisplayLine);
    }
  }
  if (InputManager::buttonWasClicked) {
    Renderer::playSound(clickToneFreq, toneDuration);
    if (optionLocked) {
      unlockCurrentOption();
      currentPointedColumn = 0;
      Renderer::lcdController.noCursor();
      Renderer::lcdController.setCursor(0, currentMenuSetup->pointedDisplayLine);
    }
    else if (currentMenuSetup->pointedOptionIndex == 0) {
      lockCurrentOption();
      currentPointedColumn = 1;
      Renderer::lcdController.setCursor(1, currentMenuSetup->pointedDisplayLine);
      Renderer::lcdController.cursor();
    }
    else if (currentMenuSetup->pointedOptionIndex == currentMenuSetup->noOptions - 1) {
      EEPROM.put(Constants::usernameAddr, *(PtrToNameArray)currentMenuSetup->options[0]);
      delete[] currentMenuSetup->options[0];
      this->onExitSubMenu();
    }
  }
}

void Menu::onDifficultySetting() {
  if (InputManager::newJoyDir && optionLocked) {
    if (InputManager::currentJoyDir == InputManager::JoyDirection::RIGHT && difficultyValue != Constants::Difficulty::HARD) {
      if (difficultyValue == Constants::Difficulty::EASY) {
        difficultyValue = Constants::Difficulty::MEDIUM;
        currentMenuSetup->options[0] = "MEDIUM: Walls, Projectiles Rain";
      }
      else {
        difficultyValue = Constants::Difficulty::HARD;
        currentMenuSetup->options[0] = "HARD: Walls, Projectiles Rain, Speed Up";
      }
      Renderer::lcdController.setCursor(1, currentMenuSetup->pointedDisplayLine);
      Renderer::lcdController.print(currentMenuSetup->options[0]);
      this->resetTextScrolling();
    }
    else if (InputManager::currentJoyDir == InputManager::JoyDirection::LEFT && difficultyValue != Constants::Difficulty::EASY) {
      if (difficultyValue == Constants::Difficulty::HARD) {
        difficultyValue = Constants::Difficulty::MEDIUM;
        currentMenuSetup->options[0] = "MEDIUM: Walls, Projectiles Rain";
      }
      else {
        difficultyValue = Constants::Difficulty::EASY;
        currentMenuSetup->options[0] = "EASY (Walls)   ";
      }
      Renderer::lcdController.setCursor(1, currentMenuSetup->pointedDisplayLine);
      Renderer::lcdController.print(currentMenuSetup->options[0]);
      this->resetTextScrolling();
    }
  }
  if (InputManager::buttonWasClicked) {
    Renderer::playSound(clickToneFreq, toneDuration);
    if (optionLocked)
      unlockCurrentOption();
    else if (currentMenuSetup->pointedOptionIndex == 0)
      lockCurrentOption();
    else if (currentMenuSetup->pointedOptionIndex == currentMenuSetup->noOptions - 1) {
      EEPROM.put(Constants::difficultyAddr, (uint8_t)difficultyValue);
      this->onExitSubMenu();
    }
  }
}

void Menu::onContrastSetting() {
  const uint8_t unitsPerBlock = 16;
  if (InputManager::newJoyDir && optionLocked) {
    char* blocksBar = (char*)currentMenuSetup->options[0];
    if (InputManager::currentJoyDir == InputManager::JoyDirection::RIGHT && currentPointedColumn < Constants::noLcdColumns) {
      blocksBar[currentPointedColumn - 1] = B11111111;  // Fills the block
      Renderer::lcdController.setCursor(currentPointedColumn++, 0);
      Renderer::lcdController.write(B11111111);
      contrastValue += unitsPerBlock;
      analogWrite(Constants::lcdContrastPin, contrastValue);
    }
    else if (InputManager::currentJoyDir == InputManager::JoyDirection::LEFT && currentPointedColumn > 1) {
      Renderer::lcdController.setCursor(--currentPointedColumn, 0);
      Renderer::lcdController.print(' ');
      blocksBar[currentPointedColumn - 1] = ' ';
      contrastValue -= unitsPerBlock;
      analogWrite(Constants::lcdContrastPin, contrastValue);
    }
  }
  if (InputManager::buttonWasClicked) {
    Renderer::playSound(clickToneFreq, toneDuration);
    if (optionLocked) {
      unlockCurrentOption();
      currentPointedColumn = 0;
    }
    else if (currentMenuSetup->pointedOptionIndex == 0) {
      lockCurrentOption();
      currentPointedColumn = (contrastValue / unitsPerBlock) + 1;
    }
    else if (currentMenuSetup->pointedOptionIndex == currentMenuSetup->noOptions - 1) {
      EEPROM.put(Constants::lcdContrastAddr, contrastValue);
      delete[] currentMenuSetup->options[0]; 
      this->onExitSubMenu();
    }
  }
}

void Menu::onLcdBrightSetting() {
  const uint8_t unitsPerBlock = 16;
  if (InputManager::newJoyDir && optionLocked) {
    char* blocksBar = (char*)currentMenuSetup->options[0];
    if (InputManager::currentJoyDir == InputManager::JoyDirection::RIGHT && currentPointedColumn < Constants::noLcdColumns) {
      blocksBar[currentPointedColumn - 1] = B11111111;  // Fills the block
      Renderer::lcdController.setCursor(currentPointedColumn++, 0);
      Renderer::lcdController.write(B11111111);
      lcdBrightValue += unitsPerBlock;
      analogWrite(Constants::lcdBrightnessPin, lcdBrightValue);
    }
    else if (InputManager::currentJoyDir == InputManager::JoyDirection::LEFT && currentPointedColumn > 1) {
      Renderer::lcdController.setCursor(--currentPointedColumn, 0);
      Renderer::lcdController.print(' ');
      blocksBar[currentPointedColumn - 1] = ' ';
      lcdBrightValue -= unitsPerBlock;
      analogWrite(Constants::lcdBrightnessPin, lcdBrightValue);
    }
  }
  if (InputManager::buttonWasClicked) {
    Renderer::playSound(clickToneFreq, toneDuration);
    if (optionLocked) {
      unlockCurrentOption();
      currentPointedColumn = 0;
    }
    else if (currentMenuSetup->pointedOptionIndex == 0) {
      lockCurrentOption();
      currentPointedColumn = (lcdBrightValue / unitsPerBlock) + 1;
    }
    else if (currentMenuSetup->pointedOptionIndex == currentMenuSetup->noOptions - 1) {
      EEPROM.put(Constants::lcdBrightAddr, lcdBrightValue);
      delete[] currentMenuSetup->options[0];
      this->onExitSubMenu();
    }
  }
}

void Menu::onMatrixBrightSetting() {
  const uint8_t unitsPerBlock = 1;
  if (InputManager::newJoyDir && optionLocked) {
    char* blocksBar = (char*)currentMenuSetup->options[0];
    if (InputManager::currentJoyDir == InputManager::JoyDirection::RIGHT && currentPointedColumn < Constants::noLcdColumns) {
      blocksBar[currentPointedColumn - 1] = B11111111;  // Fills the block
      Renderer::lcdController.setCursor(currentPointedColumn++, 0);
      Renderer::lcdController.write(B11111111);
      matrixBrightValue += unitsPerBlock;
      Renderer::setMatrixBrightness(matrixBrightValue);
    }
    else if (InputManager::currentJoyDir == InputManager::JoyDirection::LEFT && currentPointedColumn > 1) {
      Renderer::lcdController.setCursor(--currentPointedColumn, 0);
      Renderer::lcdController.print(' ');
      blocksBar[currentPointedColumn - 1] = ' ';
      matrixBrightValue -= unitsPerBlock;
      Renderer::setMatrixBrightness(matrixBrightValue);
    }
  }
  if (InputManager::buttonWasClicked) {
    Renderer::playSound(clickToneFreq, toneDuration);
    if (optionLocked) {
      unlockCurrentOption();
      currentPointedColumn = 0;
    }
    else if (currentMenuSetup->pointedOptionIndex == 0) {
      lockCurrentOption();
      currentPointedColumn = (matrixBrightValue / unitsPerBlock) + 1;
    }
    else if (currentMenuSetup->pointedOptionIndex == currentMenuSetup->noOptions - 1) {
      for (uint8_t i = 0; i < Constants::matrixSize; ++i)
        for (uint8_t j = 0; j < Constants::matrixSize; ++j)
          Renderer::addMatrixChange(i, j, LOW);
      EEPROM.put(Constants::matrixBrightAddr, matrixBrightValue);
      delete[] currentMenuSetup->options[0];
      this->onExitSubMenu();
    }
  }
}

void Menu::onMuteSoundsSetting() {
  if (InputManager::newJoyDir && optionLocked) {
    if (InputManager::currentJoyDir == InputManager::JoyDirection::RIGHT || 
        InputManager::currentJoyDir == InputManager::JoyDirection::LEFT) {
      Renderer::soundsOn = !Renderer::soundsOn;
      if (Renderer::soundsOn)
        currentMenuSetup->options[0] = "ON ";
      else
        currentMenuSetup->options[0] = "OFF";
      Renderer::lcdController.setCursor(1, currentMenuSetup->pointedDisplayLine);
      Renderer::lcdController.print(currentMenuSetup->options[0]);
    }
  }
  if (InputManager::buttonWasClicked) {
    Renderer::playSound(clickToneFreq, toneDuration);
    if (optionLocked)
      unlockCurrentOption();
    else if (currentMenuSetup->pointedOptionIndex == 0)
      lockCurrentOption();
    else if (currentMenuSetup->pointedOptionIndex == currentMenuSetup->noOptions - 1) {
      EEPROM.put(Constants::soundsOnAddr, Renderer::soundsOn);
      this->onExitSubMenu();
    }
  }
}

void Menu::onAboutOption() {
  if (InputManager::buttonWasClicked && currentMenuSetup->pointedOptionIndex == currentMenuSetup->noOptions - 1) {
    this->onExitSubMenu();
    Renderer::playSound(clickToneFreq, toneDuration);
  }
}

void Menu::checkOptionScrolling() {
  bool scrolled = false;
  if (InputManager::newJoyDir) {
    if (InputManager::currentJoyDir == InputManager::JoyDirection::UP && 
        currentMenuSetup->pointedOptionIndex > 0 && !optionLocked) {
      currentMenuSetup->pointedDisplayLine = 0;
      currentMenuSetup->pointedOptionIndex--;
      scrolled = true;
    }
    else if (InputManager::currentJoyDir == InputManager::JoyDirection::DOWN && 
        currentMenuSetup->pointedOptionIndex < currentMenuSetup->noOptions - 1 && !optionLocked) {
      currentMenuSetup->pointedDisplayLine = 1;
      currentMenuSetup->pointedOptionIndex++;
      scrolled = true;
    }
  }
  if (scrolled) {
    currentPointedColumn = 0;
    this->resetTextScrolling();
    this->displayCurrentSetup();
    Renderer::playSound(scrollToneFreq, toneDuration);  
  }
}

void Menu::checkTextScrolling() {
  if (currentOptionLength >= Constants::noLcdColumns && millis() - lastTextScrollTime >= scrollingTextSpeed) {
    ++scrollingTextIndex;
    if (scrollingTextIndex == currentOptionLength - Constants::noLcdColumns + 2)
      scrollingTextIndex = 0;
    Renderer::lcdController.setCursor(1, currentMenuSetup->pointedDisplayLine);
    Renderer::lcdController.print(currentMenuSetup->options[currentMenuSetup->pointedOptionIndex] + scrollingTextIndex);
    lastTextScrollTime = millis();
  }
}

void Menu::resetTextScrolling() {
  currentOptionLength = strlen(currentMenuSetup->options[currentMenuSetup->pointedOptionIndex]);
  lastTextScrollTime = millis();
  scrollingTextIndex = 0;
}

void Menu::checkOptionLocked() {
  if (optionLocked)
    if (millis() - lastPointerBlinkTime >= pointerBlinkDelay) {
      pointerShown = !pointerShown;
      lastPointerBlinkTime = millis();
      Renderer::lcdController.setCursor(0, currentMenuSetup->pointedDisplayLine);
      if (pointerShown)
        Renderer::lcdController.print('>');
      else
        Renderer::lcdController.print(' ');
      Renderer::lcdController.setCursor(currentPointedColumn, currentMenuSetup->pointedDisplayLine);
    }
}

void Menu::lockCurrentOption() {
  optionLocked = true;
  pointerShown = true;
  lastPointerBlinkTime = 0;
}

void Menu::unlockCurrentOption() {
  optionLocked = false;
  Renderer::lcdController.setCursor(0, currentMenuSetup->pointedDisplayLine);
  Renderer::lcdController.print('>');
  Renderer::lcdController.setCursor(currentPointedColumn, currentMenuSetup->pointedDisplayLine);
}

void Menu::displayCurrentSetup() {
  Renderer::lcdController.clear();
  if (currentMenuSetup->pointedDisplayLine == 0) {
    Renderer::lcdController.print('>');
    Renderer::lcdController.print(currentMenuSetup->options[currentMenuSetup->pointedOptionIndex] + scrollingTextIndex);
    Renderer::lcdController.setCursor(1, 1);
    Renderer::lcdController.print(currentMenuSetup->options[currentMenuSetup->pointedOptionIndex + 1]);
  }
  else {
    Renderer::lcdController.setCursor(1, 0);
    Renderer::lcdController.print(currentMenuSetup->options[currentMenuSetup->pointedOptionIndex - 1]);
    Renderer::lcdController.setCursor(0, 1);
    Renderer::lcdController.print('>');
    Renderer::lcdController.print(currentMenuSetup->options[currentMenuSetup->pointedOptionIndex] + scrollingTextIndex);
  }
}

void Menu::onEnterSubMenu(SubMenuSetup* newSetup) {
  if (stackSize < maxStackSize)
    subMenusStack[stackSize++] = currentMenuSetup;
  currentMenuSetup = newSetup;

  currentOptionLength = strlen(currentMenuSetup->options[0]);
  currentPointedColumn = 0;
  scrollingTextIndex = 0;
  lastTextScrollTime = millis();

  this->displayCurrentSetup();
}

void Menu::onExitSubMenu() {
  if (stackSize > 0) {
    delete[] currentMenuSetup->options;
    delete currentMenuSetup;
    currentMenuSetup = subMenusStack[--stackSize];
    subMenusStack[stackSize] = nullptr;
  }
  
  currentOptionLength = strlen(currentMenuSetup->options[0]);
  currentPointedColumn = 0;
  scrollingTextIndex = 0;
  lastTextScrollTime = millis();

  this->displayCurrentSetup();
}

void Menu::loadHighscores(SubMenuSetup* setupContainer) {
  uint32_t scoreValue;
  uint8_t noSavedScores;
  EEPROM.get(Constants::noSavedScoresAddr, noSavedScores);
  setupContainer->noOptions = noSavedScores + 2;  // + 2 for "Reset" and "Go back to menu"
  setupContainer->options = new const char*[setupContainer->noOptions];
  for (uint8_t i = 0; i < noSavedScores; ++i) {
    char* newHighscore = new char[Constants::usernameSize + Constants::maxScoreDigits + 2];  // + 2 for space character and null terminator in case of max size text
    setupContainer->options[i] = newHighscore;
    uint16_t currentScoreNameAddr = Constants::highscoresAddr + i * Constants::usernameSize + i * sizeof(scoreValue);
    EEPROM.get(currentScoreNameAddr, *(PtrToNameArray)newHighscore);
    EEPROM.get(currentScoreNameAddr + Constants::usernameSize, scoreValue);
    newHighscore[Constants::usernameSize] = ' ';
    ultoa(scoreValue, newHighscore + Constants::usernameSize + 1, 10);  // converts number in base 10
  }
  setupContainer->options[setupContainer->noOptions - 2] = "Reset scores";
  setupContainer->options[setupContainer->noOptions - 1] = "Go back to menu";
}

char* Menu::loadBlocksIndicator(uint8_t eepromValueAddr, uint8_t& eepromValueOut, uint8_t unitsPerBlock) {
  EEPROM.get(eepromValueAddr, eepromValueOut);
  char* blocksBar = new char[Constants::noLcdColumns];
  for (uint8_t i = 0; i < Constants::noLcdColumns - 1; ++i) {
    if (i < eepromValueOut / unitsPerBlock)
      blocksBar[i] = B11111111;  // Full block
    else
      blocksBar[i] = ' ';  // Empty block
  }
  blocksBar[Constants::noLcdColumns - 1] = '\0';
  return blocksBar;
}
