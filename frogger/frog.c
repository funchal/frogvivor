#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_mixer.h>

SDL_Surface* screen = NULL;

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

void free_image(SDL_Surface* image) {
    SDL_FreeSurface(image);
}

void draw_image(int x, int y, SDL_Surface* image) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    if(SDL_BlitSurface(image, NULL, screen, &rect) < 0) {
        int* p = 0;
        *p = 3;
        fprintf(stderr, "SDL_BlitSurface error: %s\n", SDL_GetError());
        exit(1);
    }
}

// static resources
SDL_Surface* frog[4];
SDL_Surface* jump[4]; 
SDL_Surface* car[5];
SDL_Surface* truck[2];
SDL_Surface* terrain[7];

// dynamic state
struct player {
    int position;
    int state;
    int key_pressed;
    SDLKey key;
} player[4];

void tick() {
    // background
    int i;
    for(i = 0; i != 10; ++i) {
        draw_image(0, 48*i, terrain[i % 7]);
    }

    // cars
    draw_image(48, 48, car[3]);

    // frogs
    SDL_Surface* image;
    for(i = 0; i != 4; ++i) {
        switch(player[i].state) {
            case 9:
            case 0:
                player[i].state = 0;
                image = frog[i];
                if(player[i].key_pressed) {
                    player[i].state++;
                    player[i].position++;
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

        draw_image(48*(3+2*i), 48*(9-player[i].position), image);
    }
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;
    // initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_putenv("SDL_VIDEO_CENTERED=center");
    SDL_WM_SetCaption("Frogger", "Frogger");
    screen = SDL_SetVideoMode(640, 480, 24, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if(screen == NULL) {
        fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
        exit(1);
    }

    if(Mix_OpenAudio(22040, AUDIO_S16SYS, 2, 4096) != 0) {
        fprintf(stderr, "Unable to initialize audio: %s\n", SDL_GetError());
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

    // initial state
    memset(player, 0, 4*sizeof(player));
    player[0].key = SDLK_UP;
    player[1].key = SDLK_z;
    player[2].key = SDLK_p;
    player[3].key = SDLK_q;

    int i;
    Uint32 time = 0;
    while(1) {
        // flip screen buffer
        if(SDL_Flip(screen) != 0) {
            fprintf(stderr, "Failed to swap the buffers: %s", SDL_GetError());
            exit(1);
        }

        tick();

        // event loop
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
                    break;
                case SDL_QUIT:
                    exit(0);
                    break;
                default:
                    break;
            }
        }

        // limit frames per second
        Uint32 delay = 15; // ms
        Uint32 curr = SDL_GetTicks() - time;
        if(curr < delay) {
            SDL_Delay(delay - curr);
        }
        time = SDL_GetTicks();
    }

    SDL_Quit();
    return 0;
}
