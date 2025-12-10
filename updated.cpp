#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>

using namespace std;

SDL_Texture* RenderText(SDL_Renderer* renderer, TTF_Font* font,
                        const string& text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) {
        cout << "TTF_RenderText error: " << TTF_GetError() << endl;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        cout << "SDL_CreateTextureFromSurface error: " << SDL_GetError() << endl;
    }

    return texture;
}

int RunMenu(SDL_Renderer* renderer,
            TTF_Font* font,
            TTF_Font* titleFont,
            SDL_Rect screen) {
    int selectedPlayers = 2; 

    int boxWidth  = 300;
    int boxHeight = 50;
    int gap       = 20;
    int centerX   = screen.w / 2;
    int startY    = static_cast<int>(screen.h / 2.3f);

    SDL_Rect option2     = { centerX - boxWidth / 2,          startY,
                             boxWidth, boxHeight };
    SDL_Rect option3     = { centerX - boxWidth / 2,
                             startY + (boxHeight + gap),
                             boxWidth, boxHeight };
    SDL_Rect option4     = { centerX - boxWidth / 2,
                             startY + 2 * (boxHeight + gap),
                             boxWidth, boxHeight };
    SDL_Rect continueBtn = { centerX - boxWidth / 2,
                             startY + 3 * (boxHeight + gap) + 20,
                             boxWidth, boxHeight };

    SDL_Color textColor { 60, 40, 20, 255 };

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return 0; 
            }

            if (event.type == SDL_MOUSEBUTTONDOWN &&
                event.button.button == SDL_BUTTON_LEFT) {

                SDL_Point mouse { event.button.x, event.button.y };

                if (SDL_PointInRect(&mouse, &option2)) {
                    selectedPlayers = 2;
                } else if (SDL_PointInRect(&mouse, &option3)) {
                    selectedPlayers = 3;
                } else if (SDL_PointInRect(&mouse, &option4)) {
                    selectedPlayers = 4;
                } else if (SDL_PointInRect(&mouse, &continueBtn)) {
                    return selectedPlayers; 
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 224, 189, 255);
        SDL_RenderClear(renderer);

        SDL_Texture* titleTex =
            RenderText(renderer, titleFont, "Select Number of Players", textColor);
        if (titleTex) {
            int tw, th;
            SDL_QueryTexture(titleTex, nullptr, nullptr, &tw, &th);
            SDL_Rect titleRect { centerX - tw / 2, 120, tw, th };
            SDL_RenderCopy(renderer, titleTex, nullptr, &titleRect);
            SDL_DestroyTexture(titleTex);
        }

        auto DrawOption = [&](SDL_Rect rect, int value) {
            if (selectedPlayers == value)
                SDL_SetRenderDrawColor(renderer, 210, 180, 150, 255); // selected
            else
                SDL_SetRenderDrawColor(renderer, 230, 200, 170, 255); // normal

            SDL_RenderFillRect(renderer, &rect);

            SDL_SetRenderDrawColor(renderer, 120, 80, 40, 255);
            SDL_RenderDrawRect(renderer, &rect);

            string label = to_string(value) + " Players";
            SDL_Texture* tex = RenderText(renderer, font, label, textColor);
            if (tex) {
                int w, h;
                SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
                SDL_Rect dst {
                    rect.x + rect.w / 2 - w / 2,
                    rect.y + rect.h / 2 - h / 2,
                    w, h
                };
                SDL_RenderCopy(renderer, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
            }
        };

        DrawOption(option2, 2);
        DrawOption(option3, 3);
        DrawOption(option4, 4);

        SDL_SetRenderDrawColor(renderer, 200, 160, 120, 255);
        SDL_RenderFillRect(renderer, &continueBtn);
        SDL_SetRenderDrawColor(renderer, 120, 80, 40, 255);
        SDL_RenderDrawRect(renderer, &continueBtn);

        SDL_Texture* contTex =
            RenderText(renderer, font, "Continue", textColor);
        if (contTex) {
            int tw, th;
            SDL_QueryTexture(contTex, nullptr, nullptr, &tw, &th);
            SDL_Rect contRect {
                continueBtn.x + continueBtn.w / 2 - tw / 2,
                continueBtn.y + continueBtn.h / 2 - th / 2,
                tw, th
            };
            SDL_RenderCopy(renderer, contTex, nullptr, &contRect);
            SDL_DestroyTexture(contTex);
        }

        SDL_RenderPresent(renderer);
    }

    return 0;
}

void RunGame(SDL_Renderer* renderer,
             TTF_Font* font,
             SDL_Rect screen,
             int selectedPlayers) {
    SDL_Event event;
    bool running = true;

    int boardWidth  = static_cast<int>(screen.w * 0.75f);
    int panelWidth  = screen.w - boardWidth;
    int boardX      = panelWidth;
    int boardY      = 40;
    int boardHeight = screen.h - 80;

    int cellW = boardWidth / 10;
    int cellH = boardHeight / 10;

    SDL_Color textColor { 60, 40, 20, 255 };

    vector<int> positions(selectedPlayers, 0);  
    vector<bool> finished(selectedPlayers, false);
    vector<int> finishOrder;
    int currentPlayer = 0;
    int diceValue     = 0;
    bool gameOver     = false;

    SDL_Rect rollButton {
        panelWidth / 2 - 80,
        screen.h / 2 + 40,
        160,
        60
    };

    auto GetCellRect = [&](int index) {
        int row = index / 10;
        int col = index % 10;

        int x = boardX + col * cellW;
        int y = boardY + (9 - row) * cellH; 

        SDL_Rect r { x, y, cellW, cellH };
        return r;
    };

    auto NextPlayer = [&]() {
        if (gameOver) return;
        for (int i = 0; i < selectedPlayers; ++i) {
            currentPlayer = (currentPlayer + 1) % selectedPlayers;
            if (!finished[currentPlayer])
                break;
        }
    };

    SDL_Color playerColors[4] = {
        {220,  40,  40, 255}, // P1 red
        { 40, 120, 220, 255}, // P2 blue
        { 40, 180,  80, 255}, // P3 green
        {200, 200,  40, 255}  // P4 yellow
    };

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;

            if (!gameOver &&
                event.type == SDL_MOUSEBUTTONDOWN &&
                event.button.button == SDL_BUTTON_LEFT) {

                SDL_Point mouse { event.button.x, event.button.y };

                if (SDL_PointInRect(&mouse, &rollButton) &&
                    !finished[currentPlayer]) {

                    diceValue = rand() % 6 + 1;

                    int newPos = positions[currentPlayer] + diceValue;
                    if (newPos >= 99) {
                        newPos = 99;
                        if (!finished[currentPlayer]) {
                            finished[currentPlayer] = true;
                            finishOrder.push_back(currentPlayer);
                            if ((int)finishOrder.size() == selectedPlayers) {
                                gameOver = true;
                            }
                        }
                    }
                    positions[currentPlayer] = newPos;
                    NextPlayer();
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 224, 189, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 240, 210, 180, 255);
        SDL_Rect panelRect { 0, 0, panelWidth, screen.h };
        SDL_RenderFillRect(renderer, &panelRect);

        string status = gameOver
                        ? "Game Over!"
                        : "Player " + to_string(currentPlayer + 1) + "'s turn";

        SDL_Texture* statusTex = RenderText(renderer, font, status, textColor);
        if (statusTex) {
            int sw, sh;
            SDL_QueryTexture(statusTex, nullptr, nullptr, &sw, &sh);
            SDL_Rect srect { panelWidth / 2 - sw / 2, 60, sw, sh };
            SDL_RenderCopy(renderer, statusTex, nullptr, &srect);
            SDL_DestroyTexture(statusTex);
        }

        string diceMsg = "Roll: " + (diceValue == 0 ? string("-") : to_string(diceValue));
        SDL_Texture* diceTex = RenderText(renderer, font, diceMsg, textColor);
        if (diceTex) {
            int dw, dh;
            SDL_QueryTexture(diceTex, nullptr, nullptr, &dw, &dh);
            SDL_Rect drect { panelWidth / 2 - dw / 2, 120, dw, dh };
            SDL_RenderCopy(renderer, diceTex, nullptr, &drect);
            SDL_DestroyTexture(diceTex);
        }

        int legendY = 200;
        for (int i = 0; i < selectedPlayers; ++i) {
            SDL_Color c = playerColors[i];

            SDL_Rect colorBox { 30, legendY, 25, 25 };
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
            SDL_RenderFillRect(renderer, &colorBox);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &colorBox);

            string pname = "Player " + to_string(i + 1);
            SDL_Texture* nameTex = RenderText(renderer, font, pname, textColor);
            if (nameTex) {
                int nw, nh;
                SDL_QueryTexture(nameTex, nullptr, nullptr, &nw, &nh);
                SDL_Rect nrect { 70, legendY + 1, nw, nh };
                SDL_RenderCopy(renderer, nameTex, nullptr, &nrect);
                SDL_DestroyTexture(nameTex);
            }

            legendY += 35;
        }

        SDL_SetRenderDrawColor(renderer, 200, 160, 120, 255);
        SDL_RenderFillRect(renderer, &rollButton);
        SDL_SetRenderDrawColor(renderer, 120, 80, 40, 255);
        SDL_RenderDrawRect(renderer, &rollButton);

        SDL_Texture* rollTex = RenderText(renderer, font, "Roll", textColor);
        if (rollTex) {
            int rw, rh;
            SDL_QueryTexture(rollTex, nullptr, nullptr, &rw, &rh);
            SDL_Rect rrect {
                rollButton.x + rollButton.w / 2 - rw / 2,
                rollButton.y + rollButton.h / 2 - rh / 2,
                rw, rh
            };
            SDL_RenderCopy(renderer, rollTex, nullptr, &rrect);
            SDL_DestroyTexture(rollTex);
        }

        SDL_SetRenderDrawColor(renderer, 245, 220, 190, 255);
        SDL_Rect boardRect { boardX, boardY, cellW * 10, cellH * 10 };
        SDL_RenderFillRect(renderer, &boardRect);

        SDL_SetRenderDrawColor(renderer, 120, 80, 40, 255);
        for (int i = 0; i < 100; ++i) {
            SDL_Rect cell = GetCellRect(i);
            SDL_RenderDrawRect(renderer, &cell);

            string n = to_string(i);
            SDL_Texture* numTex = RenderText(renderer, font, n, textColor);
            if (numTex) {
                int nw, nh;
                SDL_QueryTexture(numTex, nullptr, nullptr, &nw, &nh);
                SDL_Rect nrect { cell.x + 4, cell.y + 4, nw, nh };
                SDL_RenderCopy(renderer, numTex, nullptr, &nrect);
                SDL_DestroyTexture(numTex);
            }
        }

        for (int i = 0; i < selectedPlayers; ++i) {
            SDL_Rect cell = GetCellRect(positions[i]);

            int subW = cellW / 3;
            int subH = cellH / 3;

            int offsetX = (i % 2) * subW;
            int offsetY = (i / 2) * subH;

            SDL_Rect pRect {
                cell.x + offsetX + 8,
                cell.y + offsetY + 8,
                subW,
                subH
            };

            SDL_SetRenderDrawColor(renderer,
                                   playerColors[i].r,
                                   playerColors[i].g,
                                   playerColors[i].b,
                                   255);
            SDL_RenderFillRect(renderer, &pRect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &pRect);
        }

        SDL_RenderPresent(renderer);
    }
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cout << "SDL Init Error: " << SDL_GetError() << endl;
        return 1;
    }

    if (TTF_Init() != 0) {
        cout << "SDL_ttf Init Error: " << TTF_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    SDL_Rect screen;
    SDL_GetDisplayUsableBounds(0, &screen);

    SDL_Window* window = SDL_CreateWindow(
        "Snake-Ladder Dice Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        screen.w,
        screen.h,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        cout << "Window creation error: " << SDL_GetError() << endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cout << "Renderer creation error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    TTF_Font* font      = TTF_OpenFont("LiberationSans-Regular.ttf", 28);
    TTF_Font* titleFont = TTF_OpenFont("LiberationSans-Regular.ttf", 40);

    if (!font || !titleFont) {
        cout << "Failed to load font: " << TTF_GetError() << endl;
        if (font) TTF_CloseFont(font);
        if (titleFont) TTF_CloseFont(titleFont);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    int players = RunMenu(renderer, font, titleFont, screen);
    if (players > 0) {
        RunGame(renderer, font, screen, players);
    }

    TTF_CloseFont(font);
    TTF_CloseFont(titleFont);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
