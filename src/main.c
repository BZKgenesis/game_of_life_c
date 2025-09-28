#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SDL_FLAGS SDL_INIT_VIDEO

#define WINDOW_TITLE "Le jeu de la vie"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

#define GAME_SIZE 200

#define CELL_ALIVE 1
#define CELL_DEAD 0

typedef uint8_t GameTab[(GAME_SIZE*GAME_SIZE)/8 + 1];

GameTab a, b;

struct Game {
    SDL_Window *window;
    SDL_Renderer *renderer;
    GameTab *game_tab_current;
    GameTab *game_tab_next;
};

bool game_init_sdl(struct Game *g);
void game_free(struct Game *g);
void game_run(struct Game *g);
void display_game(struct Game *g);

bool isAlive(const int X, const int Y, const GameTab game) {
    const int n_byte = (X + Y*GAME_SIZE) / 8;
    const int n_bit = (X + Y*GAME_SIZE) % 8;
    return (game[n_byte] >> n_bit) & 1;
}

void setCase(const int X, const int Y, GameTab game, bool state) {
    const int n_byte = (X + Y*GAME_SIZE) / 8;
    const int n_bit = (X + Y*GAME_SIZE) % 8;
    if(state)
        game[n_byte] |= 1 << n_bit;
    else
        game[n_byte] &= ~(1 << n_bit);
}


void init_game_tab(struct Game *g) {
    g->game_tab_next = &a;
    g->game_tab_current = &b;
    srand((unsigned)time(0));
    for (int i = 0; i < GAME_SIZE; i++) {
        for (int j = 0; j < GAME_SIZE; j++) {
            const short val = rand()%2;
            setCase(i,j,(*g->game_tab_next),val);
            setCase(i,j,(*g->game_tab_current),val);
        }
    }
}

bool game_init(struct Game *g) {

    const bool return_code = game_init_sdl(g);
    init_game_tab(g);
    return return_code;

}

bool game_init_sdl(struct Game *g) {
    if (!SDL_Init(SDL_FLAGS)) {
        fprintf(stderr, "Error initializing SDL3: %s\n", SDL_GetError());
        return false;
    }

    g->window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!g->window) {
        fprintf(stderr, "Error creating Window: %s\n", SDL_GetError());
        return false;
    }

    g->renderer = SDL_CreateRenderer(g->window, NULL);
    if (!g->renderer) {
        fprintf(stderr, "Error creating Renderer: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

void game_free(struct Game *g) {
    if (g->renderer) {
        SDL_DestroyRenderer(g->renderer);
        g->renderer = NULL;
    }

    if (g->window) {
        SDL_DestroyWindow(g->window);
        g->window = NULL;
    }

    SDL_Quit();
}

void update_game(GameTab game_tab_current, GameTab game_tab_next) {
    for (int i = 0; i < GAME_SIZE; i++) {
        for (int j = 0; j < GAME_SIZE; j++) {
            int counter = 0;

            counter += isAlive((i-1+GAME_SIZE)%GAME_SIZE,(j-1+GAME_SIZE)%GAME_SIZE, game_tab_current);
            counter += isAlive((i  +GAME_SIZE)%GAME_SIZE,(j-1+GAME_SIZE)%GAME_SIZE, game_tab_current);
            counter += isAlive((i+1+GAME_SIZE)%GAME_SIZE,(j-1+GAME_SIZE)%GAME_SIZE, game_tab_current);
            counter += isAlive((i-1+GAME_SIZE)%GAME_SIZE,(j  +GAME_SIZE)%GAME_SIZE, game_tab_current);

            counter += isAlive((i+1+GAME_SIZE)%GAME_SIZE,(j  +GAME_SIZE)%GAME_SIZE, game_tab_current);
            counter += isAlive((i-1+GAME_SIZE)%GAME_SIZE,(j+1+GAME_SIZE)%GAME_SIZE, game_tab_current);
            counter += isAlive((i  +GAME_SIZE)%GAME_SIZE,(j+1+GAME_SIZE)%GAME_SIZE, game_tab_current);
            counter += isAlive((i+1+GAME_SIZE)%GAME_SIZE,(j+1+GAME_SIZE)%GAME_SIZE, game_tab_current);

            const bool alive = isAlive(i, j, game_tab_current);
            if ((alive && (counter == 2 || counter == 3)) || !alive && counter == 3)
                setCase(i, j, game_tab_next, CELL_ALIVE);
            else
                setCase(i, j, game_tab_next, CELL_DEAD);
        }
    }
}

void game_run(struct Game *g) {
    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE)// touche echap
                    running = 0;
            }
        }

        SDL_Delay(1);
        update_game(*g->game_tab_current,*g->game_tab_next);
        GameTab *tmp = g->game_tab_current; g->game_tab_current = g->game_tab_next; g->game_tab_next = tmp;

        display_game(g);

    }
}


void display_game(struct Game *g) {
    SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 255);
    SDL_RenderClear(g->renderer);
    const float case_width = (float)WINDOW_WIDTH / GAME_SIZE;
    const float case_height = (float)WINDOW_HEIGHT / GAME_SIZE;
    for (int i = 0; i < GAME_SIZE; i++) {
        for (int j = 0; j < GAME_SIZE; j++) {
            SDL_FRect rect = {(float)i*case_width, (float)j*case_height, case_width, case_height};
            if (isAlive(i,j,*g->game_tab_current)) {
                SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
            }else {
                SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 255);
            }
            SDL_RenderFillRect(g->renderer, &rect);
        }
    }
    SDL_RenderPresent(g->renderer);
}

int main(void) {
    bool exit_status = EXIT_FAILURE;

    struct Game game = {0};

    if (game_init(&game)) {
        game_run(&game);
        exit_status = EXIT_SUCCESS;
    }

    game_free(&game);

    return exit_status;
}