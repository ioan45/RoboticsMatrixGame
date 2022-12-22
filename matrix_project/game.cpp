#include <EEPROM.h>
#include "constants.h"
#include "input_manager.h"
#include "renderer.h"
#include "game.h"
#include "game_over.h"


float Game::score = 0.0f;
uint32_t Game::gameDuration = 0;

Game::Game() 
  : maxWallHeight(6),
    maxWallWidth(6), 
    stateMatrixHeight(Constants::matrixSize + maxWallHeight + 2),
    playerBlinkDelay(150)
{
}

void Game::start() {
  Renderer::clearMatrix();
  Renderer::lcdController.clear();
  Renderer::lcdController.print("Score: 0");
  Renderer::lcdController.setCursor(0, 1);
  Renderer::lcdController.print('>');
  char username[Constants::usernameSize];
  EEPROM.get(Constants::usernameAddr, username);
  Renderer::lcdController.print(username);

  randomSeed(analogRead(Constants::freeAnalogPin));
  EEPROM.get(Constants::difficultyAddr, difficultyValue);

  matrixState = new uint8_t*[stateMatrixHeight];
  for (uint8_t i = 0; i < stateMatrixHeight; ++i)
    matrixState[i] = new uint8_t[Constants::matrixSize]{0};
  
  score = 0.0f;
  if (difficultyValue == Constants::Difficulty::EASY)
    scoreIncreaseRate = 1.0f;
  else if (difficultyValue == Constants::Difficulty::MEDIUM)
    scoreIncreaseRate = 2.0f;
  else
    scoreIncreaseRate = 3.0f;

  activeObsAction = &Game::onTransition;
  matrixShiftCounter = 0;
  playerShown = true;
  shiftDelay = 450.0f;
  transStartTime = millis();
  lastShiftTime = millis();
  lastSpeedUpdateTime = millis();
  lastScoreUpdateTime = millis();
  lastPlayerBlinkTime = millis();
  gameStartTime = millis();
  playerCol = Constants::matrixSize / 2;
  Renderer::addMatrixChange(Constants::matrixSize - 1, playerCol, HIGH);
}

void Game::update() {
  checkPlayerMovement();
  if (activeObsAction)
    (this->*activeObsAction)();
  if (difficultyValue == Constants::Difficulty::HARD)
    updateObstaclesSpeed();
  updateScore();
  updatePlayerBlink();
  shiftMatrix();
  checkGameOver();
}

void Game::updatePlayerBlink() {
  if (millis() - lastPlayerBlinkTime >= playerBlinkDelay) {
    playerShown = !playerShown;
    if (playerShown)
      Renderer::addMatrixChange(Constants::matrixSize - 1, playerCol, HIGH);
    else
      Renderer::addMatrixChange(Constants::matrixSize - 1, playerCol, LOW);
    lastPlayerBlinkTime = millis();
  }
}

void Game::updateScore() {
  constexpr uint16_t scoreUpdateDelay = 500;
  if (millis() - lastScoreUpdateTime >= scoreUpdateDelay) {
    score += scoreIncreaseRate;
    Renderer::lcdController.setCursor(7, 0);
    Renderer::lcdController.print(int32_t(score));
    lastScoreUpdateTime = millis();    
  }
}

void Game::shiftMatrix() {
  if (millis() - lastShiftTime < shiftDelay)
    return;
  
  for (uint8_t i = stateMatrixHeight - 1; i > 0; --i) {
    for (uint8_t j = 0; j < Constants::matrixSize; ++j) {
      if (i >= stateMatrixHeight - Constants::matrixSize && ((matrixState[i][j] != 0) ^ (matrixState[i - 1][j] != 0))) {
        Renderer::addMatrixChange(i - (stateMatrixHeight - Constants::matrixSize), j, matrixState[i - 1][j]);
      }
      matrixState[i][j] = matrixState[i - 1][j];
    }
  }
  for (uint8_t i = 0; i < Constants::matrixSize; ++i)
    matrixState[0][i] = 0;
  Renderer::addMatrixChange(Constants::matrixSize - 1, playerCol, HIGH);
  lastShiftTime = millis();
  ++matrixShiftCounter;
}

void Game::checkGameOver() {
  constexpr uint16_t gameOverToneFreq = 300;
  constexpr uint16_t gameOverToneDur = 800;
  if (matrixState[stateMatrixHeight - 1][playerCol] > 0) {
    for (uint8_t i = 0; i < stateMatrixHeight; ++i)
      delete[] matrixState[i];
    delete[] matrixState;
    gameDuration = (millis() - gameStartTime) / 1000;
    Renderer::playSound(gameOverToneFreq, gameOverToneDur);
    Scene::playNewScene<GameOver>();
  }
}

void Game::checkPlayerMovement() {
  constexpr uint16_t moveToneFreq = 625;
  constexpr uint16_t moveToneDur = 50;
  if (InputManager::newJoyDir) {
    if (InputManager::currentJoyDir == InputManager::JoyDirection::LEFT && playerCol > 0) {
      Renderer::addMatrixChange(Constants::matrixSize - 1, playerCol, LOW);
      --playerCol;
      Renderer::addMatrixChange(Constants::matrixSize - 1, playerCol, HIGH);
      playerShown = true;
      Renderer::playSound(moveToneFreq, moveToneDur);
    }
    else if (InputManager::currentJoyDir == InputManager::JoyDirection::RIGHT && playerCol < Constants::matrixSize - 1) {
      Renderer::addMatrixChange(Constants::matrixSize - 1, playerCol, LOW);
      ++playerCol;
      Renderer::addMatrixChange(Constants::matrixSize - 1, playerCol, HIGH);
      playerShown = true;
      Renderer::playSound(moveToneFreq, moveToneDur);
    }
  }
}

void Game::updateObstaclesSpeed() {
  constexpr uint16_t minShiftDelay = 170;  // Minimum delay for shifting down obstacles on led matrix (ms).
  constexpr uint16_t shiftDelayChangeRate = 10;  // The rate at which shifting delay will decrease over time (ms/s).
  constexpr float msToS = 0.001f;  // Convert milliseconds to seconds.
  if (shiftDelay > minShiftDelay) {
    shiftDelay -= shiftDelayChangeRate * ((millis() - lastSpeedUpdateTime) * msToS);
    lastSpeedUpdateTime = millis();
  }
}

void Game::onTransition() {
  constexpr uint16_t transDuration = 2000;
  if (millis() - transStartTime < transDuration)
    return;
  
  constexpr uint8_t maxRandValue = 100;
  uint8_t randomValue = random() % maxRandValue;
  if (difficultyValue == Constants::Difficulty::EASY || randomValue < maxRandValue / 2) {
    noWallsCreated = 0;
    matrixShiftCounter = maxWallHeight + 1;
    activeObsAction = &Game::onWalls;
  }
  else {
    projWavesLaunched = 0;
    activeObsAction = &Game::onProjectilesRain;
  }
}

void Game::onWalls() {
  constexpr uint8_t maxWallsPerWave = 8;
  if (matrixShiftCounter >= stateMatrixHeight) {
    transStartTime = millis();
    activeObsAction = &Game::onTransition;
    return;
  }
  if (matrixShiftCounter < maxWallHeight + 2)
    return;
  if (noWallsCreated == maxWallsPerWave)
    return;

  constexpr uint8_t noWallTypes = 5;
  uint8_t randomWall = random() % noWallTypes;
  uint8_t randomColumn = random() % (Constants::matrixSize - maxWallWidth + 1);
  if (randomWall == 0) {
    uint8_t wallMatrixPtr[maxWallHeight][maxWallWidth] = {
      {1, 0, 0, 0, 0, 0},
      {0, 1, 0, 0, 0, 0},
      {0, 0, 1, 0, 0, 0},
      {0, 0, 0, 1, 0, 0},
      {0, 0, 0, 0, 1, 0},
      {0, 0, 0, 0, 0, 1}
    };
    for (uint8_t i = 0; i < maxWallHeight; ++i)
      for (uint8_t j = randomColumn; j < randomColumn + maxWallWidth; ++j)
        matrixState[i][j] = wallMatrixPtr[i][j - randomColumn];
  }
  else if (randomWall == 1) {
    uint8_t wallMatrixPtr[maxWallHeight][maxWallWidth] = {
      {0, 0, 0, 0, 0, 1},
      {0, 0, 0, 0, 1, 0},
      {0, 0, 0, 1, 0, 0},
      {0, 0, 1, 0, 0, 0},
      {0, 1, 0, 0, 0, 0},
      {1, 0, 0, 0, 0, 0}
    };
    for (uint8_t i = 0; i < maxWallHeight; ++i)
      for (uint8_t j = randomColumn; j < randomColumn + maxWallWidth; ++j)
        matrixState[i][j] = wallMatrixPtr[i][j - randomColumn];
  }
  else if (randomWall == 2) {
    uint8_t wallMatrixPtr[maxWallHeight][maxWallWidth] = {
      {0, 1, 0, 0, 0, 0},
      {0, 0, 1, 0, 0, 0},
      {0, 0, 1, 0, 0, 0},
      {0, 0, 0, 1, 0, 0},
      {0, 0, 0, 1, 0, 0},
      {0, 0, 0, 0, 1, 0}
    };
    for (uint8_t i = 0; i < maxWallHeight; ++i)
      for (uint8_t j = randomColumn; j < randomColumn + maxWallWidth; ++j)
        matrixState[i][j] = wallMatrixPtr[i][j - randomColumn];
  }
  else if (randomWall == 3) {
    uint8_t wallMatrixPtr[maxWallHeight][maxWallWidth] = {
      {0, 0, 0, 0, 1, 0},
      {0, 0, 0, 1, 0, 0},
      {0, 0, 0, 1, 0, 0},
      {0, 0, 1, 0, 0, 0},
      {0, 0, 1, 0, 0, 0},
      {0, 1, 0, 0, 0, 0}
    };
    for (uint8_t i = 0; i < maxWallHeight; ++i)
      for (uint8_t j = randomColumn; j < randomColumn + maxWallWidth; ++j)
        matrixState[i][j] = wallMatrixPtr[i][j - randomColumn];
  }
  else if (randomWall == 4) {
    uint8_t wallMatrixPtr[maxWallHeight][maxWallWidth] = {
      {0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0},
      {1, 1, 1, 1, 1, 1},
      {0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0}
    };
    for (uint8_t i = 0; i < maxWallHeight; ++i)
      for (uint8_t j = randomColumn; j < randomColumn + maxWallWidth; ++j)
        matrixState[i][j] = wallMatrixPtr[i][j - randomColumn];
  }
  ++noWallsCreated;
  matrixShiftCounter = 0;  
}

void Game::onProjectilesRain() {
  constexpr uint8_t maxProjWavesLaunched = 40;
  if (matrixShiftCounter >= Constants::matrixSize) {
    transStartTime = millis();
    activeObsAction = &Game::onTransition;
    return;
  }
  if (projWavesLaunched == maxProjWavesLaunched)
    return;
  if (matrixShiftCounter == 0)
    return;

  constexpr uint8_t maxProjOnWave = 2;
  uint8_t projToLaunchOnWave = 1 + (random() % maxProjOnWave);
  for (uint8_t i = 0; i < projToLaunchOnWave; ++i) {
    uint8_t randomProjCol = random() % Constants::matrixSize;
    matrixState[stateMatrixHeight - Constants::matrixSize][randomProjCol] = 1;
    Renderer::addMatrixChange(0, randomProjCol, HIGH);
  }
  ++projWavesLaunched;
  matrixShiftCounter = 0;
}