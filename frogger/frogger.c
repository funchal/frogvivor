#include <SDL.h>
#include <SDL_mixer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

// uncomment to get raw background and more speed
//#define PRETTY_BACKGROUND 1

// screen dimentions
#define SCREEN_HEIGHT 480
#define SCREEN_WIDTH 640

// number of pixel per band
#define NB_PIXELS_PER_LINE 48

// offset from the left side of the screen to the first vertical line.
// note: the first vertical line is partially displayed out of the screen.
#define OFFSET_FIRST_VERTICAL_LINE (-40)

// number of grass/road bands before finish line
#define FINISH_BAND_NUMBER 20
// total number of grass/road bands 
// (before finish line + after finish line + 1 for display facilities)
#define NB_BANDS (FINISH_BAND_NUMBER + 3 + 1)
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
SDL_Surface* frog[5]; // 4 colors + 1 grey
SDL_Surface* jump[5]; // 4 colors + 1 grey 
SDL_Surface* splat[5]; // 4 colors + 1 grey
SDL_Surface* car[4];
SDL_Surface* truck[4];
SDL_Surface* carRL[5];
SDL_Surface* truckRL[4];
SDL_Surface* grass;
SDL_Surface* road;
SDL_Surface* font;

Mix_Chunk* croak;

// dynamic state
struct player {
    int position;
    int state;
    bool key_pressed;
    SDLKey key;
    int x; // x coordinate
    bool alive;
    bool on_finish_line;
    SDL_Surface* image;
    int score;
    bool AI; // true iff artificial intelligence is playing
    bool AI_go_up; // true iff AI wants to jump upwards
} player[4];

typedef enum { CAR = 0, TRUCK } vehicle_type;

struct vehicle {
    vehicle_type type;
    int color; // between 0 and 3 if car
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
} background[NB_BANDS];


enum { INIT, PICK, GAME, WINNER } game_state;

// max row allowed for frogs. (frogs must not go up out of the screen)
int max_row_allowed;

// min row allowed for frogs. (frogs must not disapear when screen scroll up)
int min_row_allowed;

// screen offset in pixels
int offset;

// for some parts of the code we want reproducible random
// we store the current seed before these parts and restore it after
int random_seed;

SDL_Surface* load_image(const char* filename, Uint8 hue) {
    SDL_Surface* temp;
    temp = SDL_LoadBMP(filename);
    if(temp == NULL) {
        fprintf(stderr, "Could not load %s: %s\n", filename, SDL_GetError());
        exit(1);
    }
    SDL_Surface* image = SDL_DisplayFormat(temp);
    SDL_FreeSurface(temp);
    SDL_SetColorKey(image, SDL_SRCCOLORKEY, SDL_MapRGB(image->format, 0, 0, 0));
    if(hue != 0) {
    SDL_LockSurface(image);
    int i;
    hue=hue/42;
    for(i = 0; i != image->w * image->h; ++i) {
        Uint8* data = (Uint8*) image->pixels;
        Uint8* r = &data[3*i+2];
        Uint8* g = &data[3*i+1];
        Uint8* b = &data[3*i];
        Uint8 p = (*r * *r)/256;
        Uint8 q = sqrt(255.0 * *r);
        switch(hue) {
            case 0: *r = q; *g = p; *b = p; break;
            case 1: *r = q; *g = q; *b = p; break;
            case 2: *r = p; *g = q; *b = p; break;
            case 3: *r = p; *g = q; *b = q; break;
            case 4: *r = p; *g = p; *b = q; break;
            case 5: *r = q; *g = p; *b = q; break;
            default: break;
        }
    }
    SDL_UnlockSurface(image);
    }
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
    Uint8* pixmem = (Uint8*) screen->pixels + 3*(SCREEN_WIDTH*y + x);

    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return;
    }

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
    max_row_allowed = 8;
    min_row_allowed = 0;
    offset = 0;

    int i, j;
    memset(background, 0, sizeof(background));
    if (NB_BANDS < LAUNCH_PAD_SIZE) {
        exit(1);
    }

    for (i = 0 ; i < LAUNCH_PAD_SIZE ; ++i) {
        background[i].road = false;
    }

    for (i = LAUNCH_PAD_SIZE ; i < FINISH_BAND_NUMBER ; ++i) {
        // fixme: change probability (for the time 1/2)
        background[i].road = get_random(0, 1);
        background[i].direction = get_random(0, 1) ? LR : RL;
        // generate vehicles
        background[i].speed = get_random(0, 1) ? SLOW : FAST;
        background[i].nb_vehicles = get_random(0, 1) ? 1 : 2;
        for (j = 0 ; j <= 1 ; j++) {
            // type: car or truck
            if (get_random(0, 1)) {
                background[i].veh[j].type = CAR;
                background[i].veh[j].length = CAR_LENGTH;
            }
            else {
                background[i].veh[j].type = TRUCK;
                background[i].veh[j].length = TRUCK_LENGTH;
            }
            // color: from 0 to 3
            background[i].veh[j].color = get_random(0, 3);
        }
        // first vehicle coordinate
        background[i].veh[0].x = get_random(0, 1000 - 1);
        // second vehicle must not overlap
        int sum_veh_lengths = background[i].veh[0].length + background[i].veh[0].length;
        int distance_between_vehicles = get_random(0, 1000 - 1 - sum_veh_lengths - 2*MIN_DISTANCE_BETWEEN_VEHICLES);
        background[i].veh[1].x = background[i].veh[0].x + background[i].veh[0].length + distance_between_vehicles + MIN_DISTANCE_BETWEEN_VEHICLES;
    }

    for (i = FINISH_BAND_NUMBER ; i < NB_BANDS ; ++i) {
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

// choice should depend on the prensence of vehicles
// on the current road and next road(s),
// and possibly on position wrt the other frogs
void update_AI_choice(int i) {
    int probability_to_go_up; // draw within 1000 possibilities

    if (background[player[i].position + 1].road == false) {
        // next band is grass
        if (background[player[i].position].road == false) {
            // current band is grass
            probability_to_go_up = 80;
        }
        else {
            // current band is a road
            probability_to_go_up = 100;
        }
    }
    else { 
        // next band is a road
        if (background[player[i].position].road == false) {
            // current band is grass
            probability_to_go_up = 60;
        }
        else {
            // current band is a road
            probability_to_go_up = 80;
        }
    }

    if (get_random(0, 1000) <= probability_to_go_up) {
        player[i].AI_go_up = true;
    }
    else {
        player[i].AI_go_up = false;
    }
}

void next_player_state(int i) {
    int i_veh;
    bool collision;
    int veh_length;
    int row;
    int max_row_allowed_for_this_player;

    switch(player[i].state) {
    case 9:
    case 0:
        player[i].state = 0;
        if (player[i].AI) {
            update_AI_choice(i);
            player[i].image = frog[4]; // grey, no color
        }
        else {
            player[i].image = frog[i];
        }
        if (game_state == PICK) {
            max_row_allowed_for_this_player = player[i].AI ? 0 : 1;
        }
        else {
            max_row_allowed_for_this_player = max_row_allowed;
        }
        if( // normal jump
           (!player[i].AI && player[i].key_pressed && (player[i].position < max_row_allowed_for_this_player)) ||
           // normal jump for AI
           (player[i].AI && player[i].AI_go_up && (player[i].position < max_row_allowed_for_this_player)) ||
           // emergency jump
           (player[i].position < min_row_allowed) )
            {
                player[i].state++;
                player[i].position++;
                Mix_PlayChannel(-1, croak, 0);
            }
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        if (player[i].AI) {
            player[i].image = jump[4]; // grey, no color
        }
        else { 
            player[i].image = jump[i];
        }
        player[i].state++;
                break;
    case 5:
    case 6:
    case 7:
    case 8:
        if (player[i].AI) {
            player[i].image = frog[4]; // grey, no color
        }
        else {
            player[i].image = frog[i];
        }
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
        if (player[i].AI) {
            player[i].image = splat[4];
        }
        else {
            player[i].image = splat[i];
        }
    }
    if (player[i].position == FINISH_BAND_NUMBER) {
        player[i].on_finish_line = true;
    }
}

void draw_one_road_band(int y, int line_number) {
    int i, j;
    SDL_LockSurface(screen);
#ifdef PRETTY_BACKGROUND
    random_seed = random();
    srandom(line_number*42);

    for (i = 0 ; i < SCREEN_WIDTH ; ++i) {
        for (j = 0 ; j < NB_PIXELS_PER_LINE ; ++j) {
            // dotted white line
            if ((i>>5)%2==0 && (j == 2 || j == NB_PIXELS_PER_LINE-3)) {
                set_pixel(i, y+j, 255, 255, 255);
            }
            // normal grey
            else {
                int x = get_random(0, 5);
                switch(x) {
                case 0:
                    set_pixel(i, y+j, 135, 135, 135);
                    break;
                case 1:
                    set_pixel(i, y+j, 120, 120, 120);
                    break;
                case 2:
                    set_pixel(i, y+j, 125, 125, 125);
                    break;
                case 3:
                case 4:
                case 5:
                    set_pixel(i, y+j, 130, 130, 130);
                    break;
                default:
                    break;
                }
            }
        }
    }
    srandom(random_seed);
#else
    for (i = 0 ; i < SCREEN_WIDTH ; ++i) {
        for (j = 0 ; j < NB_PIXELS_PER_LINE ; ++j) {
            // dotted white line
            if ((i>>5)%2==0 && (j == 2 || j == NB_PIXELS_PER_LINE-3)) {
                set_pixel(i, y+j, 255, 255, 255);
            }
            // normal grey
            else {
                set_pixel(i, y+j, 135, 135, 135);
            }
        }
    }
#endif
    SDL_UnlockSurface(screen);
}

void draw_one_grass_band(int y, int line_number) {
    int i, j;
    SDL_LockSurface(screen);
#ifdef PRETTY_BACKGROUND
    random_seed = random();
    srandom(line_number*42);

    for (i = 0 ; i < SCREEN_WIDTH ; ++i) {
        for (j = 0 ; j < NB_PIXELS_PER_LINE ; ++j) {
            int x = get_random(0, 5);
            switch(x) {
                case 0:
                    set_pixel(i, y+j, 0x6e, 0x89, 0x4f);
                    break;
                case 1:
                    set_pixel(i, y+j, 0x4c, 0x65, 0x51);
                    break;
                case 2:
                    set_pixel(i, y+j, 0x31, 0x4b, 0x00);
                    break;
                case 3:
                case 4:
                case 5:
                    set_pixel(i, y+j, 0x0e, 0x21, 0x00);
                    break;
                default:
                    break;
            }
        }
    }
    srandom(random_seed);
#else
    for (i = 0 ; i < SCREEN_WIDTH ; ++i) {
        for (j = 0 ; j < NB_PIXELS_PER_LINE ; ++j) {
            set_pixel(i, y+j, 0x31, 0x4b, 0x00);
        }
    }
#endif
    SDL_UnlockSurface(screen);
}

void draw_background() {
    // improvement? generate roads only when needed and forget about old roads. 11-cell tab is enough
    int i, i_veh;
    char buffer[60];

    for(i = 0; i != 11; ++i) {
        // distance from the top of the screen
        // y = (total height) - (piece of first line) - (full lines between 1 and i)
        int y = SCREEN_HEIGHT - (NB_PIXELS_PER_LINE-(offset%NB_PIXELS_PER_LINE)) - (i*NB_PIXELS_PER_LINE);
        int line_number = (offset/NB_PIXELS_PER_LINE) + i;
        // draw background
        if (background[line_number].road == true) { // road
            draw_one_road_band(y, line_number);
        }
        else { // grass
            draw_one_grass_band(y, line_number);
        }
        if (line_number == FINISH_BAND_NUMBER && y >= 0) { // finish line is on the top (not lower nor higher!)
            sprintf(buffer, "FINISH");
            draw_text(268, y+16, buffer);
        }                
    }
}

void draw_frogs() {
    int nb_alive = 0;
    int frog_alive = 100; // one of living frog
    int nb_finish = 0;
    int frog_finish = 100; // one of living frog
    int i;

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
        game_state = WINNER;
    }
    else if (nb_alive == 1) { // one winner
        printf("frog %d wins!\n", frog_alive);
        game_state = WINNER;
        player[frog_alive].score++;
    }
    else if (nb_finish == 1) { // one winner
        printf("frog %d wins!\n", frog_finish);
        game_state = WINNER;
        player[frog_finish].score++;
    }
    else if (nb_finish > 1) { // draw
        printf("draw\n");
        game_state = WINNER;
    }

    for(i = 0; i != 4; ++i) {
        draw_image(player[i].x, NB_PIXELS_PER_LINE*(9-player[i].position)+offset, player[i].image);
    }
}

void draw_vehicles() {
    int i, i_veh;
    SDL_Surface* veh_image;
    for(i = 0; i != 11; ++i) {
        // distance from the top of the screen
        // y = (total height) - (piece of first line) - (full lines between 1 and i)
        int y = SCREEN_HEIGHT - (NB_PIXELS_PER_LINE-(offset%NB_PIXELS_PER_LINE)) - (i*NB_PIXELS_PER_LINE);
        int line_number = (offset/NB_PIXELS_PER_LINE) + i;
                
        if (background[line_number].road == true) { // road
            for (i_veh = 0 ; i_veh < background[line_number].nb_vehicles ; ++i_veh) {
                int color = background[line_number].veh[i_veh].color;
                if (background[line_number].veh[i_veh].type == CAR) {
                    if (background[line_number].direction == LR) {
                        veh_image = car[color];
                    }
                    else {
                        veh_image = carRL[color];
                    }
                }
                else {
                    if (background[line_number].direction == LR) {
                        veh_image = truck[color];
                    }
                    else {
                        veh_image = truckRL[color];
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
}

int pick_count;

void tick() {
    switch(game_state) {
        case INIT: {
            int i;
            for (i=0 ; i < 4 ; i++) {
                player[i].position = 0;
                player[i].state = 0;
                player[i].key_pressed = false;
                player[i].alive = true;
                player[i].x = 48*(4+2*i) + OFFSET_FIRST_VERTICAL_LINE;
                player[i].image = frog[4]; // grey until the player decides to play
                player[i].on_finish_line = false;
                player[i].AI = true;
                player[i].AI_go_up = false;
            }

            generate_background();

            pick_count = 0;
            game_state = PICK;
            }
            break;
        case PICK: {
            int i;

            // background
            draw_background();

            draw_text(200, 220, "PICK YOUR FROG!");
            pick_count++;
            if(pick_count == 200) {
                game_state = GAME;
                pick_count = 0;
            }

            for (i = 0 ; i < 4 ; ++i) {
                if (player[i].key_pressed && player[i].AI) {
                    // note: from now on, the frog is colorized
                    player[i].AI = false;
                }
                next_player_state(i);
                draw_image(player[i].x, NB_PIXELS_PER_LINE*(9-player[i].position)+offset, player[i].image);
            }
        }

            break;
        case GAME: {
            int i;
            // scrolling
            int max_row = 0; // the upper frog is at row maxRow
            for (i = 0 ; i != 4 ; ++i) {
                if(player[i].alive && player[i].position > max_row) {
                    max_row = player[i].position;
                }
            }
            if ((// a frog is near the top of the screen
                 ((max_row-3)*NB_PIXELS_PER_LINE > offset) ||
                 // the bottom band has not completely disappeared
                 (offset%48 != 0)) 
                &&
                // the last line is not visible yet
                (offset < (NB_BANDS)*NB_PIXELS_PER_LINE - SCREEN_HEIGHT)) {
                offset++;
            }
            max_row_allowed = (offset+SCREEN_HEIGHT)/NB_PIXELS_PER_LINE;
            min_row_allowed = (offset+NB_PIXELS_PER_LINE/2)/NB_PIXELS_PER_LINE;

            // background
            draw_background();

            // compute next state of frogs and display them
            draw_frogs();

            // compute next state of vehicles and display them
            draw_vehicles();

            // text
            char buffer[60];
            sprintf(buffer, "%2d    %2d    %2d    %2d", player[0].score, player[1].score, player[2].score, player[3].score);
            draw_text(152, SCREEN_HEIGHT-24, buffer);
        }
            break;
        case WINNER:
            draw_text(300, 220, "WINNER!");
            pick_count++;
            if(pick_count == 200) {
                game_state = INIT;
            }
            break;
        default:
            break;
    }
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;
    int i;

    // initialise random
    srandom(42);

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
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 24, SDL_HWSURFACE | SDL_DOUBLEBUF);
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
    frog[0] = load_image("images/50.bmp", 170);
    frog[1] = load_image("images/50.bmp", 213);
    frog[2] = load_image("images/50.bmp", 42);
    frog[3] = load_image("images/50.bmp", 85);
    frog[4] = load_image("images/50.bmp", 0);

    jump[0] = load_image("images/51.bmp", 170);
    jump[1] = load_image("images/51.bmp", 213);
    jump[2] = load_image("images/51.bmp", 42);
    jump[3] = load_image("images/51.bmp", 85);
    jump[4] = load_image("images/51.bmp", 0);

    splat[0] = load_image("images/splat.bmp", 170);
    splat[1] = load_image("images/splat.bmp", 213);
    splat[2] = load_image("images/splat.bmp", 42);
    splat[3] = load_image("images/splat.bmp", 85);
    splat[4] = load_image("images/splat.bmp", 0);

    car[0] = load_image("images/103.bmp", 0);
    car[1] = load_image("images/103.bmp", 0);
    car[2] = load_image("images/103.bmp", 0);
    car[3] = load_image("images/103.bmp", 0);

    truck[0] = load_image("images/105.bmp", 0);
    truck[1] = load_image("images/105.bmp", 0);
    truck[2] = load_image("images/105.bmp", 0);
    truck[3] = load_image("images/105.bmp", 0);

    carRL[0] = load_image("images/113.bmp", 0);
    carRL[1] = load_image("images/113.bmp", 0);
    carRL[2] = load_image("images/113.bmp", 0);
    carRL[3] = load_image("images/113.bmp", 0);

    truckRL[0] = load_image("images/115.bmp", 0);
    truckRL[1] = load_image("images/115.bmp", 0);
    truckRL[2] = load_image("images/115.bmp", 0);
    truckRL[3] = load_image("images/115.bmp", 0);

    grass = load_image("images/200.bmp", 0);
    road  = load_image("images/201.bmp", 0);
    font  = load_image("images/font.bmp", 0);

    croak = Mix_LoadWAV("sounds/4.wav");

    game_state = INIT;

    // main synchronous loop
    bool quit = false;
    Uint32 time = 0;
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

    for(i = 0; i != 5; ++i) {
        SDL_FreeSurface(frog[i]);
        SDL_FreeSurface(jump[i]);
    }

    for(i = 0; i != 4; ++i) {
        SDL_FreeSurface(car[i]);
    }

    for(i = 0; i != 4; ++i) {
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
