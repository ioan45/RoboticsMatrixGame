#include "input_manager.h"
#include "renderer.h"
#include "scene.h"
#include "greetings.h"

void setup() {
  InputManager::init();
  Renderer::init();
  Scene::playNewScene<Greetings>();
}

void loop() {
  InputManager::getUserInput();
  Scene::changeIfRequested();
  Scene::getCurrentScene()->update();
  Renderer::updateMatrix();
}
