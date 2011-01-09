#include <SDL.h>
#include <SDL_mixer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// number of pixel per band
#define NB_PIXELS_PER_LINE 48

// offset from the left side of the screen to the first vertical line.
// note: the first vertical line is partially displayed out of the screen.
#define OFFSET_FIRST_VERTICAL_LINE (-40)

// number of grass/road bands before finish line
#define NB_BANDS 15

// minimum number of grass bands before first road 
#define LAUNCH_PAD_SIZE 3

// minimum distance between two vehicles on the same road
#define MIN_DISTANCE_BETWEEN_VEHICLES 96

// LENGTH OF A FROG, A CAR, A TRUCK
#define FROG_LENGTH 48
#define CAR_LENGTH 96
#define TRUCK_LENGTH 144

// static resources
SDL_Surface* screen;
SDL_Surface* frog[4];
SDL_Surface* jump[4]; 
SDL_Surface* splat[4];
SDL_Surface* car[4];
SDL_Surface* truck[2];
SDL_Surface* carRL[5];
SDL_Surface* truckRL[2];
SDL_Surface* grass;
SDL_Surface* road;
SDL_Surface* font;

Mix_Chunk* croak;

// dynamic state
struct player {
    int position;
    int state;
    int key_pressed;
    SDLKey key;
    int x; // x coordinate
    bool alive;
    bool on_finish_line;
    SDL_Surface* image;
    int score;
} player[4];

typedef enum { CAR = 0, TRUCK } vehicle_type;

struct vehicle {
    vehicle_type type;
    // x coordinate over 1000 pixels
    // 1000 px is larger than the screen width because vehicules rotate
    // screen goes from pixel 180 to pixel 1000-180-1
    int x; 
    // horizontal length
    int length;
};

typedef enum { SLOW = 0, FAST } vehicle_speed;
typedef enum { LR = 0, RL } road_direction; // from left to right or vice-versa

struct band {
    bool road; // road or grass
    road_direction direction;
    vehicle_speed speed; // speed of the vehicules
    int nb_vehicles; // 1 or 2
    struct vehicle veh[2]; // depending on nb_vehicules, 1 or 2 are used
} // NB_BANDS-1 normal road/grass bands + 1 finish band + 1 band for easy display
    background[NB_BANDS+1];


// the upper frog is at row maxRow
int max_row;

// max row allowed for frogs. (frogs must not go up out of the screen)
int max_row_allowed;

// min row allowed for frogs. (frogs must not disapear when screen scroll up)
int min_row_allowed;

// screen offset in pixels
int offset;

// true iff one frog has just won
int end_of_race;

// true iff the user want to close the application
int quit;

SDL_Surface* load_image(const char* filename, Uint8 R, Uint8 G, Uint8 B) {
    SDL_Surface* temp;
    temp = SDL_LoadBMP(filename);
    if(temp == NULL) {
        fprintf(stderr, "Could not load %s: %s\n", filename, SDL_GetError());
        exit(1);
    }
    SDL_Surface* image = SDL_DisplayFormat(temp);
    SDL_FreeSurface(temp);
    SDL_SetColorKey(image, SDL_SRCCOLORKEY, SDL_MapRGB(image->format, 0, 0, 0));
    SDL_LockSurface(image);
    int i;
    for(i = 0; i != image->w * image->h; ++i) {
        Uint8* data = (Uint8*) image->pixels;
        Uint8* r = &data[3*i];
        Uint8* g = &data[3*i+1];
        Uint8* b = &data[3*i+2];
        if(*r < 200) {
        *r *= 1-R/255.0;
        *g *= 1-G/255.0;
        *b *= 1-B/255.0;
        }
    }
    SDL_UnlockSurface(image);
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
    int i, j;
    memset(background, 0, sizeof(background));
    if (NB_BANDS < LAUNCH_PAD_SIZE) {
        exit(1);
    }

    for (i = 0 ; i < LAUNCH_PAD_SIZE ; ++i) {
        background[i].road = false;
    }

    for (i = LAUNCH_PAD_SIZE ; i < NB_BANDS-1 ; ++i) {
        // fixme: change probability (for the time 1/2)
        background[i].road = get_random(0, 1);
        background[i].direction = get_random(0, 1) ? LR : RL;
        // generate vehicles
        background[i].speed = get_random(0, 1) ? SLOW : FAST;
        background[i].nb_vehicles = get_random(0, 1) ? 1 : 2;
        for (j = 0 ; j <= 1 ; j++) {
            if (get_random(0, 1)) {
                background[i].veh[j].type = CAR;
                background[i].veh[j].length = CAR_LENGTH;
            }
            else {
                background[i].veh[j].type = TRUCK;
                background[i].veh[j].length = TRUCK_LENGTH;
            }
        }
        // first vehicle coordinate
        background[i].veh[0].x = get_random(0, 1000 - 1);
        // second vehicle must not overlap
        int sum_veh_lengths = background[i].veh[0].length + background[i].veh[0].length;
        int distance_between_vehicles = get_random(0, 1000 - 1 - sum_veh_lengths - 2*MIN_DISTANCE_BETWEEN_VEHICLES);
        background[i].veh[1].x = background[i].veh[0].x + background[i].veh[0].length + distance_between_vehicles + MIN_DISTANCE_BETWEEN_VEHICLES;
    }

    for (i = NB_BANDS-1 ; i <= NB_BANDS ; ++i) {
        background[i].road = false;
    }
}

bool overlap(int ax, int al, int bx, int bl) {
    if ((bx > ax+al) || (bx+bl < ax)) {
        return false;
    }
    else {
        return true;
    }
}

void next_player_state(int i) {
    int i_veh;
    bool collision;
    int veh_length;
    int row;

    switch(player[i].state) {
    case 9:
    case 0:
        player[i].state = 0;
        player[i].image = frog[i];
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
        player[i].image = jump[i];
        player[i].state++;
                break;
    case 5:
    case 6:
    case 7:
    case 8:
        player[i].image = frog[i];
        player[i].state++;
        break;
    default:
        abort();
    }

    // collision
    collision = false;
    row = player[i].position;
    if (background[row].road) {
        for (i_veh = 0 ; i_veh < background[row].nb_vehicles ; ++i_veh) {
            veh_length = background[row].veh[i_veh].length;
            // -180 because x coord of vehicles is staggered with 180 pixels
            if (overlap(player[i].x, FROG_LENGTH, background[row].veh[i_veh].x-180, veh_length)) {
                collision = true;
            }
            
        }
    }
    if (collision) {
        player[i].alive = false;
    }
    if (player[i].position == NB_BANDS-1) {
        player[i].on_finish_line = true;
    }
}

void tick() {
    // scrolling
    if (// a frog is near the top of the screen
        ((max_row-3)*NB_PIXELS_PER_LINE > offset) &&
        // the finish line is not visible yet
        (offset < (NB_BANDS+1)*NB_PIXELS_PER_LINE - 480 - NB_PIXELS_PER_LINE)) {
        offset++;
    }
    max_row_allowed = (offset+480)/NB_PIXELS_PER_LINE;
    min_row_allowed = (offset+NB_PIXELS_PER_LINE/2)/NB_PIXELS_PER_LINE;

    // background
    // improvement? generate roads only when needed and forget about old roads. 11-cell tab is enough
    SDL_Surface* background_image;
    SDL_Surface* veh_image;
    int i, i_veh;
    int nb_alive;
    int frog_alive; // one of living frog
    int nb_finish;
    int frog_finish; // one of the frog on the finish line
    char buffer[60];

    for(i = 0; i != 11; ++i) {
        // distance from the top of the screen
        // y = (total height) - (piece of first line) - (full lines between 1 and i)
        int y = 480 - (NB_PIXELS_PER_LINE-(offset%NB_PIXELS_PER_LINE)) - (i*NB_PIXELS_PER_LINE);
        int line_number = (offset/NB_PIXELS_PER_LINE) + i;
        // draw background
        if (line_number == NB_BANDS-1) { // finish line
            background_image = grass;
        }
        else if (background[line_number].road == true) { // road
            background_image = road;
        }
        else { // grass
            background_image = grass;
        }
        draw_image(0, y, background_image);
        if (line_number == NB_BANDS-1) { // finish line
            sprintf(buffer, "FINISH");
            draw_text(268, 16, buffer);
        }                
    }

    // frogs
    nb_alive = 0;
    frog_alive = 100; // one of living frog
    nb_finish = 0;
    frog_finish = 100; // one of living frog
    for(i = 0; i != 4; ++i) {
        if (player[i].alive) {
            next_player_state(i);
            nb_alive++;
            frog_alive = i;
            if (player[i].on_finish_line) {
                nb_finish++;
                frog_finish = i;
            }
        }
    }

    // test end of race and give points
    if (nb_alive == 0) { // draw: no point
        printf("draw\n");
        end_of_race = true;
    }
    else if (nb_alive == 1) { // one winner
        printf("frog %d wins!\n", frog_alive);
        end_of_race = true;
        player[frog_alive].score++;
    }
    else if (nb_finish == 1) { // one winner
        printf("frog %d wins!\n", frog_finish);
        end_of_race = true;
        player[frog_finish].score++;
    }
    else if (nb_finish > 1) { // draw
        printf("draw\n");
        end_of_race = true;
    }

    for(i = 0; i != 4; ++i) {
        if (player[i].alive) {
            draw_image(player[i].x, NB_PIXELS_PER_LINE*(9-player[i].position)+offset, player[i].image);
        }
        else {
            draw_image(player[i].x, NB_PIXELS_PER_LINE*(9-player[i].position)+offset, splat[i]);
        }
    }

    // draw vehicles
    for(i = 0; i != 11; ++i) {
        // distance from the top of the screen
        // y = (total height) - (piece of first line) - (full lines between 1 and i)
        int y = 480 - (NB_PIXELS_PER_LINE-(offset%NB_PIXELS_PER_LINE)) - (i*NB_PIXELS_PER_LINE);
        int line_number = (offset/NB_PIXELS_PER_LINE) + i;
        
        if (background[line_number].road == true) { // road
            for (i_veh = 0 ; i_veh < background[line_number].nb_vehicles ; ++i_veh) {
                if (background[line_number].veh[i_veh].type == CAR) {
                    if (background[line_number].direction == LR) {
                        veh_image = car[3];
                    }
                    else {
                        veh_image = carRL[3];
                    }
                }
                else {
                    if (background[line_number].direction == LR) {
                        veh_image = truck[0];
                    }
                    else {
                        veh_image = truckRL[0];
                    }
                }

                // calculate speed
                int speed;
                if (background[line_number].speed == SLOW) {
                    speed = 2;
                }
                else {
                    speed = 3;
                }

                // update x coordinate
                if (background[line_number].direction == LR) {
                    background[line_number].veh[i_veh].x += speed;
                }
                else {
                    background[line_number].veh[i_veh].x -= speed;
                }

                // wrap cars
                background[line_number].veh[i_veh].x %= 1000;
                if(background[line_number].veh[i_veh].x < 0) {
                    background[line_number].veh[i_veh].x += 1000;
                }

                // draw cars
                draw_image(background[line_number].veh[i_veh].x-180, y, veh_image);
            }
        }
    }

    // text
    sprintf(buffer, "       %2d    %2d    %2d    %2d", player[0].score, player[1].score, player[2].score, player[3].score);
    draw_text(40, 480-24, buffer);


}

void play_one_race() {
    Uint32 time = 0;
    int i;

    generate_background();

    max_row = 0;
    max_row_allowed = 8;
    min_row_allowed = 0;
    offset = 0;
    end_of_race = false;

    for (i=0 ; i < 4 ; i++) {
        player[i].position = 0;
        player[i].state = 0;
        player[i].key_pressed = false;
        player[i].alive = true;
        player[i].x = 48*(4+2*i) + OFFSET_FIRST_VERTICAL_LINE;
        player[i].image = frog[i];
        player[i].on_finish_line = false;
    }

    while(!quit && !end_of_race) {

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
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;
    int i;

    // initialise random
    srandom(86712);

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

    // initial state
    memset(player, 0, sizeof(player));
    player[0].key = SDLK_UP;
    player[1].key = SDLK_z;
    player[2].key = SDLK_p;
    player[3].key = SDLK_q;

    // load static resources
    frog[0] = load_image("images/50.bmp", 0, 128, 0);
    frog[1] = load_image("images/50.bmp", 0, 0, 128);
    frog[2] = load_image("images/50.bmp", 128, 0, 0);
    frog[3] = load_image("images/50.bmp", 0, 128, 128);

    jump[0] = load_image("images/51.bmp", 0, 128, 0);
    jump[1] = load_image("images/51.bmp", 0, 0, 128);
    jump[2] = load_image("images/51.bmp", 128, 0, 0);
    jump[3] = load_image("images/51.bmp", 0, 128, 128);

    splat[0] = load_image("images/splat.bmp", 0, 128, 0);
    splat[1] = load_image("images/splat.bmp", 0, 0, 128);
    splat[2] = load_image("images/splat.bmp", 128, 0, 0);
    splat[3] = load_image("images/splat.bmp", 0, 128, 128);

    car[0] = load_image("images/103.bmp", 0, 128, 0);
    car[1] = load_image("images/103.bmp", 0, 0, 128);
    car[2] = load_image("images/103.bmp", 128, 0, 0);
    car[3] = load_image("images/103.bmp", 0, 128, 128);

    truck[0] = load_image("images/105.bmp", 0, 128, 0);
    truck[1] = load_image("images/105.bmp", 0, 0, 128);
    truck[2] = load_image("images/105.bmp", 128, 0, 0);
    truck[3] = load_image("images/105.bmp", 0, 128, 128);

    carRL[0] = load_image("images/113.bmp", 0, 128, 0);
    carRL[1] = load_image("images/113.bmp", 0, 0, 128);
    carRL[2] = load_image("images/113.bmp", 128, 0, 0);
    carRL[3] = load_image("images/113.bmp", 0, 128, 128);

    truckRL[0] = load_image("images/115.bmp", 0, 128, 0);
    truckRL[1] = load_image("images/115.bmp", 0, 0, 128);
    truckRL[2] = load_image("images/115.bmp", 128, 0, 0);
    truckRL[3] = load_image("images/115.bmp", 0, 128, 128);

    grass = load_image("images/200.bmp", 0, 0, 0);
    road  = load_image("images/201.bmp", 0, 0, 0);
    font  = load_image("images/font.bmp", 0, 0, 0);

    croak = Mix_LoadWAV("sounds/4.wav");

    for(i = 0; i != 4; ++i) {
        player[i].score = 0;
    }

    // main synchronous loop
    quit = 0;
    while (!quit) {
        // todo: print "ready" ... "go"

        // play
        play_one_race();

    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);

    for(i = 0; i != 4; ++i) {
        SDL_FreeSurface(frog[i]);
        SDL_FreeSurface(jump[i]);
    }

    for(i = 0; i != 4; ++i) {
        SDL_FreeSurface(car[i]);
    }

    for(i = 0; i != 2; ++i) {
        SDL_FreeSurface(truck[i]);
    }

    SDL_FreeSurface(grass);
    SDL_FreeSurface(road);
    SDL_FreeSurface(font);

    Mix_CloseAudio();
    Mix_FreeChunk(croak);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    SDL_Quit();
    return 0;
}
