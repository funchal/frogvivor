#include <SDL.h>
#include <SDL_mixer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
//#include <rand.h>

// number of pixel per band
#define NB_PIXELS_PER_LINE 48

// number of grass/road bands before finish line
#define NB_BANDS 15

// minimum number of grass bands before first road 
#define LAUNCH_PAD_SIZE 3

// static resources
SDL_Surface* screen;
SDL_Surface* frog[4];
SDL_Surface* jump[4]; 
SDL_Surface* car[5];
SDL_Surface* truck[2];
SDL_Surface* terrain[7];
SDL_Surface* font;

Mix_Chunk* croak;

// dynamic state
struct player {
    int position;
    int state;
    int key_pressed;
    SDLKey key;
} player[4];

typedef enum { CAR = 0, TRUCK } vehicle_type;

struct vehicle {
    vehicle_type type;
    int x; // x coordinate
};

typedef enum { SLOW = 0, FAST } vehicle_speed;

struct band {
    bool road; // road or grass
    vehicle_speed speed; // speed of the vehicules
    int nb_vehicles; // 1 or 2
    struct vehicle veh[2]; // depending on nb_vehicules, 1 or 2 are used
} background[NB_BANDS];


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

inline void set_pixel(int x, int y, int r, int g, int b) {
    Uint8* pixmem = (Uint8*) screen->pixels + 3*(640*y + x);
    pixmem[0] = r;
    pixmem[1] = g;
    pixmem[2] = b;
}

void draw_text(int x, int y, const char* text) {
    SDL_LockSurface(screen);
    int my;
    for(my = 0; my != 16; ++my) {
        unsigned c;
        for(c = 0; c != strlen(text); ++c) {
            if(text[c] < 32 || text[c] >= 96) {
                fprintf(stderr, "invalid character!\n");
                exit(1);
            }
            // upper left pixel in font
            Uint8* data = (Uint8*) font->pixels + 3*8*8*(text[c] - 32);

            int mx;
            for(mx = 0; mx != 16; ++mx) {
                if(data[3*((my/2)*8 + mx/2)] != 0) {
                    set_pixel(x+16*c+mx, y+my, 16*c+mx, 16*my, 32*c);
                }
            }
        }
    }
    SDL_UnlockSurface(screen);
}

long int get_random(long int min, long int max) {
    return min + random() / (RAND_MAX / (max - min + 1) + 1);
}

// generate background and vehicles
void generate_background() {
    int i, j, veh_length;
    memset(background, 0, sizeof(background));
    if (NB_BANDS < LAUNCH_PAD_SIZE) {
        exit(1);
    }
    for (i = 0 ; i < LAUNCH_PAD_SIZE ; ++i) {
        background[i].road = false;
    }
    for (i = LAUNCH_PAD_SIZE ; i < NB_BANDS ; ++i) {
        // fixme: change probability (for the time 1/2)
        background[i].road = get_random(0, 1);
        // generate vehicles
        background[i].speed = get_random(0, 1) ? SLOW : FAST;
        if (get_random(0, 1)) {
            background[i].nb_vehicles = 1;
        }
        else {
            background[i].nb_vehicles = 2;
        }
        for (j = 0 ; j <= 1 ; j++) {
            background[i].veh[j].type = get_random(0, 1) ? CAR : TRUCK;
            background[i].veh[j].x = get_random(0, 599);
        }
        // vehicles must not overlap
        for (j = 0 ; j <= 1 ; j++) {
            if (background[i].veh[1].type == CAR) {
                veh_length = 80;
            }
            else { // trunk
                veh_length = 130;
            }
            if (background[i].veh[j].x + veh_length > background[i].veh[(j+1)%2].x) {
                background[i].veh[(j+1)%2].x = (background[i].veh[j].x + veh_length) % 600;
            }
        }
    }
}

void tick() {
    // scrolling
    if (// a frog is near the top of the screen
        ((max_row-3)*NB_PIXELS_PER_LINE > offset) &&
        // the finish line is not visible yet
        (offset < (NB_BANDS+1)*NB_PIXELS_PER_LINE - 480)) {
        offset++;
    }
    max_row_allowed = (offset+480)/NB_PIXELS_PER_LINE;
    min_row_allowed = (offset+NB_PIXELS_PER_LINE/2)/NB_PIXELS_PER_LINE;

    // background
    // improvement? generate roads only when needed and forget about old roads. 11-cell tab is enough
    SDL_Surface* background_image;
    SDL_Surface* veh_image;
    int i;
    int i_veh;
    int line_number; 
    int y; // distance from the top of the screen

    for(i = 0; i != 11; ++i) {
        // y = (total height) - (piece of first line) - (full lines between 1 and i)
        y = 480 - (NB_PIXELS_PER_LINE-(offset%NB_PIXELS_PER_LINE)) - (i*NB_PIXELS_PER_LINE);
        line_number = (offset/NB_PIXELS_PER_LINE) + i;
        // draw background
        if (line_number == NB_BANDS) { // finish line
            background_image = terrain[6];
        }
        else if (background[line_number].road == true) { // road
            background_image = terrain[1];
        }
        else { // grass
            background_image = terrain[0];
        }
        draw_image(0, y, background_image);
                
        // draw vehicles
        if (background[line_number].road == true) { // road
            for (i_veh = 0 ; i_veh < background[line_number].nb_vehicles ; ++i_veh) {
                if (background[line_number].veh[i_veh].type == CAR) {
                    veh_image = car[0];
                }
                else {
                    veh_image = truck[0];
                }
                draw_image(background[line_number].veh[i_veh].x, y+3, veh_image);
                // update x coordinate
                // 800 == screen length + 200
                // + 150 ... - 150    for gradual display at the beginning of the line
                int speed;
                if (background[line_number].speed == SLOW) {
                    speed = 2;
                }
                else {
                    speed = 3;
                }
                fprintf(stderr, "%d %d %d\n", line_number, i_veh,  background[line_number].nb_vehicles);
                background[line_number].veh[i_veh].x = ((background[line_number].veh[i_veh].x + speed + 150) % 800) - 150;
            }
        }
    }

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

    // text
    draw_text(32, 16, "PLAYER 1 PLAYER 2 PLAYER 3 PLAYER 4");
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

    font = load_image("images/font.bmp");

    croak = Mix_LoadWAV("sounds/4.wav");

    // initial state
    memset(player, 0, sizeof(player));
    player[0].key = SDLK_UP;
    player[1].key = SDLK_z;
    player[2].key = SDLK_p;
    player[3].key = SDLK_q;

    // generate background
    srandom(867);
    generate_background();

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

    SDL_FreeSurface(font);

    Mix_CloseAudio();
    Mix_FreeChunk(croak);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    SDL_Quit();
    return 0;
}
