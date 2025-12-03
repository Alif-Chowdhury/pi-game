#include <SDL2/SDL.h>
#include <iostream>

int main() {
    // Initialize SDL video
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << "\n";
        return 1;
    }

    // Create fullscreen window
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

    bool running = true;
    SDL_Event event;

    // Simple main loop
    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                // Press ESC to quit
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }

        // Clear screen with a dark blue color
        SDL_SetRenderDrawColor(renderer, 10, 20, 60, 255);
        SDL_RenderClear(renderer);

        // Draw a simple rectangle so you see something
        SDL_Rect rect;
        rect.x = 200;
        rect.y = 120;
        rect.w = 400;
        rect.h = 240;

        SDL_SetRenderDrawColor(renderer, 200, 200, 50, 255);
        SDL_RenderFillRect(renderer, &rect);

        // Show everything
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
