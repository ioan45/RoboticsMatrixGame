#pragma once

#include "constants.h"
#include "scene.h"

class Game : Scene {
  private:
    using ObstaclesAction = void (Game::*)();

  public:
    void start();
    void update();
    Game();
  
  private:
    void checkGameOver();
    void updateObstaclesSpeed();
    void checkPlayerMovement();
    void onTransition();
    void onWalls();
    void onProjectilesRain();
    void shiftMatrix();
    void updateScore();
    void updatePlayerBlink();

  public:
    static float score;
    static uint32_t gameDuration;
  
  private:
    ObstaclesAction activeObsAction;
    Constants::Difficulty difficultyValue;
    const uint8_t maxWallHeight;
    const uint8_t maxWallWidth;
    const uint8_t stateMatrixHeight;
    const uint16_t playerBlinkDelay;
    uint8_t** matrixState;
    uint8_t playerLin, playerCol;
    uint8_t matrixShiftCounter;
    uint8_t noWallsCreated;
    uint8_t projWavesLaunched;
    uint8_t playerShown;
    uint32_t lastSpeedUpdateTime;
    uint32_t transStartTime;
    uint32_t lastShiftTime;
    uint32_t lastScoreUpdateTime;
    uint32_t lastPlayerBlinkTime;
    uint32_t gameStartTime;
    float scoreIncreaseRate;
    float shiftDelay;
};