#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>

int main(int argc, char* argv[]) {
    enum { none, monochrome, grayscale } filetype = none;

    if(argc == 4) {
        if(strcmp(argv[1], "--monochrome") == 0) {
            filetype = monochrome;
        }
        else if(strcmp(argv[1], "--grayscale") == 0) {
            filetype = grayscale;
        }
    }

    if(filetype == none) {
        fprintf(stderr, "Usage: %s [--monochrome/--grayscale] [input.bmp] [output.coe]\n", argv[0]);
        exit(1);
    }

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Surface* image = SDL_LoadBMP(argv[2]);
    if(image == NULL) {
        fprintf(stderr, "Could not load %s: %s\n", argv[2], SDL_GetError());
        exit(1);
    }

    SDL_Palette palette;
    SDL_PixelFormat format;
    memset(&format, 0, sizeof(SDL_PixelFormat));
    format.BitsPerPixel = 8;
    format.BytesPerPixel = 1;
    format.palette = &palette;

    switch(filetype) {
        case monochrome: {
                palette.ncolors = 2;
                palette.colors = malloc(2*sizeof(SDL_Color));
                palette.colors[0].r = 0;
                palette.colors[0].g = 0;
                palette.colors[0].b = 0;
                palette.colors[1].r = 255;
                palette.colors[1].g = 255;
                palette.colors[1].b = 255;
            }
            break;
        case grayscale: {
                palette.ncolors = 256;
                palette.colors = malloc(256*sizeof(SDL_Color));
                int i;
                for(i = 0; i != 255; i++) {
                    palette.colors[i].r = i;
                    palette.colors[i].g = i;
                    palette.colors[i].b = i;
                }
                
            }
            break;
        default:
            abort();
            break;
    }

    SDL_Surface* data = SDL_ConvertSurface(image, &format, SDL_SWSURFACE);
    if(data == NULL) {
        fprintf(stderr, "Conversion error: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_FreeSurface(image);

    FILE* output = fopen(argv[3], "w");
    if(!output) {
        fprintf(stderr, "Could not write to file %s\n", argv[3]);
    }

    fprintf(output, "memory_initialization_radix = 16;\n");
    fprintf(output, "memory_initialization_vector = ");

    int bytes = 0;

    SDL_LockSurface(data);
    switch(filetype) {
        case monochrome: {
                int j;
                for(j = 0; j != data->w * data->h; j+=8) {
                    int x = 0;
                    int i;
                    for(i = 0; i != 8; ++i) {
                        x |= ((Uint8 *)data->pixels)[j+i] << (8-i);
                    }
                    fprintf(output, "%02X", x);
                    bytes++;
                    if(j != data->w * data->h - 8) {
                        fprintf(output, ", ");
                    }
                }
            }
            break;
        case grayscale: {
                int i;
                for(i = 0; i != data->w * data->h; ++i) {
                    int x = ((Uint8 *)data->pixels)[i];
                    fprintf(output, "%02X", x);
                    bytes++;
                    if(i != data->w * data->h - 1) {
                        fprintf(output, ", ");
                    }
                }
            }
            break;
        default:
            abort();
            break;
    }
    SDL_UnlockSurface(data);

    fprintf(output, ";\n");
    fclose(output);

    printf("%s => %s ", argv[2], argv[3]);
    printf("(%s, ", filetype == monochrome ? "monochrome" : "grayscale");
    printf("%ix%i pixels, ", data->w, data->h);
    printf("%i bytes)\n", bytes);

    SDL_FreeSurface(data);
    SDL_Quit();
    return 0;
}
