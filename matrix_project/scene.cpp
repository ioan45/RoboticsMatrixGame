#include "scene.h"

Scene* Scene::currentScene = nullptr;
Scene::SceneCreator Scene::newSceneRequest = nullptr; 

Scene* Scene::getCurrentScene() {
  return currentScene;
}

void Scene::changeIfRequested() {
  if (newSceneRequest) {
    if (currentScene)
      delete(currentScene);
    currentScene = newSceneRequest();
    currentScene->start();
    newSceneRequest = nullptr;
  }
}

Scene::Scene() = default;
Scene::~Scene() = default;
