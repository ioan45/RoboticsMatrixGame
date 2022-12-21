#pragma once

class Scene {
  private:
    using SceneCreator = Scene* (*)();

  public:
    template<typename SceneType> static void playNewScene();  
    static void changeIfRequested();
    static Scene* getCurrentScene();
    
    virtual void start() = 0;
    virtual void update() = 0;
    
    virtual ~Scene();
    Scene();

  private:
    static Scene* currentScene;
    static SceneCreator newSceneRequest;
};

template<typename SceneType> 
void Scene::playNewScene() {
  newSceneRequest = [](){ return (Scene*)(new SceneType()); };
}
