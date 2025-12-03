#include <SDL2/SDL.h>
#include <iostream>

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "My Pi Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 480,
        SDL_WINDOW_FULLSCREEN_DESKTOP
    );

    if (!window) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (!renderer) {
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Get actual window size (whatever the display really is)
    int winW = 0, winH = 0;
    SDL_GetRendererOutputSize(renderer, &winW, &winH);
    std::cout << "Display size from SDL: " << winW << " x " << winH << std::endl;

    // Rectangle centered on screen
    SDL_Rect rect;
    rect.w = winW / 2;      // half the width
    rect.h = winH / 3;      // one-third the height
    rect.x = (winW - rect.w) / 2;
    rect.y = (winH - rect.h) / 2;

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                // ESC or Q to exit
                if (event.key.keysym.sym == SDLK_ESCAPE ||
                    event.key.keysym.sym == SDLK_q) {
                    running = false;
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                // tap / click anywhere to exit
                running = false;
            }
        }

        // Background (dark blue)
        SDL_SetRenderDrawColor(renderer, 10, 20, 60, 255);
        SDL_RenderClear(renderer);

        // Center rectangle (yellow)
        SDL_SetRenderDrawColor(renderer, 230, 230, 60, 255);
        SDL_RenderFillRect(renderer, &rect);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
