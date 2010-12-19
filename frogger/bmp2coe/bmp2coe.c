#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>

int main(int argc, char* argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s [input.bmp] [output.coe]\n", argv[0]);
        exit(1);
    }

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Surface* image = SDL_LoadBMP(argv[1]);
    if(image == NULL) {
        fprintf(stderr, "Could not load %s: %s\n", argv[1], SDL_GetError());
        exit(1);
    }

    SDL_Color colors[2] = { {0, 0, 0, 0}, {255, 255, 255, 255} };
    SDL_Palette palette;
    palette.ncolors = 2;
    palette.colors = colors;
    SDL_PixelFormat format;
    memset(&format, 0, sizeof(SDL_PixelFormat));
    format.palette = &palette;
    format.BitsPerPixel = 8;
    format.BytesPerPixel = 1;
    SDL_Surface* data = SDL_ConvertSurface(image, &format, SDL_SWSURFACE);
    if(data == NULL) {
        fprintf(stderr, "Conversion error: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_FreeSurface(image);

    FILE* output = fopen(argv[2], "w");
    if(!output) {
        fprintf(stderr, "Could not write to file %s\n", argv[2]);
    }

    fprintf(output, "memory_initialization_radix = 16;\n");
    fprintf(output, "memory_initialization_vector = ");

    SDL_LockSurface(data);
    int j;
    for(j = 0; j != data->w * data->h; j+=8) {
        int x = 0;
        int i;
        for(i = 0; i != 8; ++i) {
            x |= ((Uint8 *)data->pixels)[j+i] << (8-i);
        }
        fprintf(output, "%02X", x);
        if(j != data->w * data->h - 8) {
            fprintf(output, ", ");
        }
    }
    SDL_UnlockSurface(data);

    fprintf(output, ";\n");
    fclose(output);

    SDL_FreeSurface(data);
    SDL_Quit();
    return 0;
}
