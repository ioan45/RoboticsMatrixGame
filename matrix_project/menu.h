#pragma once

#include "constants.h"
#include "scene.h"

class Menu : Scene {
  private:
    using MenuAction = void (Menu::*)();
    using PtrToNameArray = char(*)[Constants::usernameSize];  // Allows EEPROM get() / put() to be used with a heap array.

    struct SubMenuSetup {
      uint8_t pointedOptionIndex = 0;
      uint8_t pointedDisplayLine = 0;
      uint8_t noOptions = 0;
      const char** options = nullptr;
      MenuAction loopLogic = nullptr;
    };
  
  public:
    void start();
    void update();
    Menu();
    ~Menu();

  private:
    void onMainMenu();
    void onHowToPlayOption();
    void onHighscoresOption();
    void onSettingsOption();
    void onEnterNameSetting();
    void onDifficultySetting();
    void onContrastSetting();
    void onLcdBrightSetting();
    void onMatrixBrightSetting();
    void onMuteSoundsSetting();
    void onAboutOption();
    void checkOptionScrolling();
    void checkTextScrolling();
    void resetTextScrolling();
    void checkOptionLocked();
    void lockCurrentOption();
    void unlockCurrentOption();
    void displayCurrentSetup();
    void onEnterSubMenu(SubMenuSetup* newSetup);
    void onExitSubMenu();
    void loadHighscores(SubMenuSetup* setupContainer);
    char* loadBlocksIndicator(uint8_t eepromValueAddr, uint8_t& eepromValueOut, uint8_t unitsPerBlock);
  
private:
    SubMenuSetup* currentMenuSetup;
    Constants::Difficulty difficultyValue;
    const uint8_t toneDuration;
    const uint16_t scrollingTextSpeed;
    const uint16_t pointerBlinkDelay;
    const uint16_t scrollToneFreq;
    const uint16_t clickToneFreq;
    uint8_t currentOptionLength;
    uint8_t currentPointedColumn;
    uint8_t scrollingTextIndex;
    uint32_t lastTextScrollTime;
    uint32_t lastPointerBlinkTime;
    uint8_t contrastValue;
    uint8_t lcdBrightValue;
    uint8_t matrixBrightValue;
    bool optionLocked;
    bool pointerShown;

    const uint8_t maxStackSize;
    uint8_t stackSize;
    SubMenuSetup** subMenusStack;
};
