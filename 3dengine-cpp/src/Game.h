#ifndef GAME_H
#define GAME_H

class Game {
    private:
        bool isRunning;
        SDL_Window *window;
        //std::vector<GameObject> gameObjects;

    public:
        Game();
        ~Game();
        void Initialize(bool isFullScreeen);
        void ProcessInput();
        void Update();
        void Render();
        void Destroy();
        bool IsRunning() const;
        int ticksLastFrame = 0;
        static SDL_Renderer *renderer;
};


#endif
