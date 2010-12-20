#include <SDL.h>
#include <SDL_mixer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// static resources
SDL_Surface* screen;
SDL_Surface* frog[4];
SDL_Surface* jump[4]; 
SDL_Surface* car[5];
SDL_Surface* truck[2];
SDL_Surface* terrain[7];
Mix_Chunk* croak;

// dynamic state
struct player {
    int position;
    int state;
    int key_pressed;
    SDLKey key;
} player[4];

#define NB_PIXELS_PER_LINE 48

// the upper frog is at row maxRow
int max_row = 0;
// max row allowed for frogs. (frogs must not go up out of the screen)
int max_row_allowed = 8;
// min row allowed for frogs. (frogs must not disapear when screen scroll up)
int min_row_allowed = 0;
// screen offset in pixels
int offset = 0;

SDL_Surface* load_image(const char* filename) {
    SDL_Surface* temp;
    temp = SDL_LoadBMP(filename);
    if(temp == NULL) {
        fprintf(stderr, "Could not load %s: %s\n", filename, SDL_GetError());
        exit(1);
    }
    SDL_Surface* image = SDL_DisplayFormat(temp);
    SDL_FreeSurface(temp);
    SDL_SetColorKey(image, SDL_SRCCOLORKEY, SDL_MapRGB(image->format, 255, 0, 255));
    return image;
}

void draw_image(int x, int y, SDL_Surface* image) {
    SDL_Rect rect = { .x = x, .y = y };
    if(SDL_BlitSurface(image, NULL, screen, &rect) < 0) {
        int* p = 0;
        *p = 3;
        fprintf(stderr, "SDL_BlitSurface error: %s\n", SDL_GetError());
        exit(1);
    }
}

void tick() {
    // scrolling
    if ((max_row-3)*NB_PIXELS_PER_LINE > offset) {
        offset++;
    }
    max_row_allowed = (offset+480)/NB_PIXELS_PER_LINE;
    min_row_allowed = (offset+NB_PIXELS_PER_LINE/2)/NB_PIXELS_PER_LINE;

    // background
    int i;
    for(i = 0; i != 11; ++i) {
        // no need to understand this. Will be changed to get random road and grass bands.
        draw_image(0, (NB_PIXELS_PER_LINE*i+offset)%(480+NB_PIXELS_PER_LINE)-NB_PIXELS_PER_LINE, terrain[i % 7]);
    }

    // cars
    // draw_image(48, 48, car[3]);

    // frogs
    SDL_Surface* image;
    for(i = 0; i != 4; ++i) {
        switch(player[i].state) {
            case 9:
            case 0:
                player[i].state = 0;
                image = frog[i];
                if( // normal jump
                    (player[i].key_pressed && (player[i].position < max_row_allowed)) ||
                    // emergency jump
                    (player[i].position < min_row_allowed) )
                {
                    player[i].state++;
                    player[i].position++;
                    Mix_PlayChannel(-1, croak, 0);
                    if(player[i].position > max_row) {
                        max_row = player[i].position;
                    }
                }
                break;
            case 1:
            case 2:
            case 3:
            case 4:
                image = jump[i];
                player[i].state++;
                break;
            case 5:
            case 6:
            case 7:
            case 8:
                image = frog[i];
                player[i].state++;
                break;
            default:
                abort();
        }

        draw_image(48*(3+2*i), NB_PIXELS_PER_LINE*(9-player[i].position)+offset, image);
    }
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    // initialize SDL
    if(SDL_Init(0) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    // initialize audio
    if(SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "Unable to initialize audio: %s\n", SDL_GetError());
        exit(1);
    }

    if(Mix_OpenAudio(22040, AUDIO_S16SYS, 2, 4096) != 0) {
        fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
        exit(1);
    }

    if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Unable to initialize video: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_putenv("SDL_VIDEO_CENTERED=center");
    SDL_WM_SetCaption("Frogger", "Frogger");
    screen = SDL_SetVideoMode(640, 480, 24, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if(screen == NULL) {
        fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
        exit(1);
    }

    // load static resources
    frog[0] = load_image("images/50.bmp");
    frog[1] = load_image("images/52.bmp");
    frog[2] = load_image("images/54.bmp");
    frog[3] = load_image("images/56.bmp");

    jump[0] = load_image("images/51.bmp");
    jump[1] = load_image("images/53.bmp");
    jump[2] = load_image("images/55.bmp");
    jump[3] = load_image("images/57.bmp");

    car[0] = load_image("images/100.bmp");
    car[1] = load_image("images/101.bmp");
    car[2] = load_image("images/102.bmp");
    car[3] = load_image("images/103.bmp");
    car[4] = load_image("images/104.bmp");

    truck[0] = load_image("images/105.bmp");
    truck[1] = load_image("images/106.bmp");

    terrain[0] = load_image("images/200.bmp");
    terrain[1] = load_image("images/201.bmp");
    terrain[2] = load_image("images/202.bmp");
    terrain[3] = load_image("images/203.bmp");
    terrain[4] = load_image("images/204.bmp");
    terrain[5] = load_image("images/205.bmp");
    terrain[6] = load_image("images/206.bmp");

    croak = Mix_LoadWAV("sounds/4.wav");

    // initial state
    memset(player, 0, sizeof(player));
    player[0].key = SDLK_UP;
    player[1].key = SDLK_z;
    player[2].key = SDLK_p;
    player[3].key = SDLK_q;

    // main synchronous loop
    int i;
    Uint32 time = 0;
    int quit = 0;
    while(!quit) {

        // limit frames per second
        Uint32 delay = 13; // ms
        Uint32 curr = SDL_GetTicks() - time;
        if(curr < delay) {
            SDL_Delay(delay - curr);
        }
        time = SDL_GetTicks();

        // calculate next frame
        tick();

        // flip screen buffer
        if(SDL_Flip(screen) != 0) {
            fprintf(stderr, "Failed to swap the buffers: %s", SDL_GetError());
            exit(1);
        }

        // handle asynchronous events
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYUP:
                    for(i = 0; i != 4; ++i) {
                        if(event.key.keysym.sym == player[i].key) {
                            player[i].key_pressed = 0;
                        }
                    }
                    break;
                case SDL_KEYDOWN:
                    for(i = 0; i != 4; ++i) {
                        if(event.key.keysym.sym == player[i].key) {
                            player[i].key_pressed = 1;
                        }
                    }
                    if(event.key.keysym.sym == SDLK_ESCAPE) {
                        quit = 1;
                    }
                    break;
                case SDL_QUIT:
                    quit = 1;
                default:
                    break;
            }
        }
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);

    for(i = 0; i != 4; ++i) {
        SDL_FreeSurface(frog[i]);
        SDL_FreeSurface(jump[i]);
    }

    for(i = 0; i != 5; ++i) {
        SDL_FreeSurface(car[i]);
    }

    for(i = 0; i != 2; ++i) {
        SDL_FreeSurface(truck[i]);
    }

    for(i = 0; i != 7; ++i) {
        SDL_FreeSurface(terrain[i]);
    }

    Mix_CloseAudio();
    Mix_FreeChunk(croak);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    SDL_Quit();
    return 0;
}
