#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>

const char* progname;

void usage() {
    fprintf(stderr, "Usage: %s [--monochrome/--grayscale] [input.bmp] [output.coe]\n", progname);
    exit(1);
}

void print_byte(FILE* output, int* bytes, int x) {
    if(*bytes != 0) {
        if(*bytes % 8 == 0) {
            fprintf(output, ",\n");
        }
        else {
            fprintf(output, ", ");
        }
    }
    (*bytes)++;
    fprintf(output, "%02X", x);
}

int main(int argc, char* argv[]) {
    enum { none, monochrome, grayscale } filetype = none;

    progname = argv[0];
    if(argc < 4) {
        usage();
    }

    if(strcmp(argv[1], "--monochrome") == 0) {
        filetype = monochrome;
    }
    else if(strcmp(argv[1], "--grayscale") == 0) {
        filetype = grayscale;
    }
    else {
        usage();
    }

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    FILE* output = fopen(argv[argc-1], "w");
    if(!output) {
        fprintf(stderr, "Could not write to file %s\n", argv[argc-1]);
    }

    fprintf(output, "memory_initialization_radix = 16;\n");
    fprintf(output, "memory_initialization_vector =\n");

    int total = 0;

    int f;
    for(f = 2; f != argc - 1; ++f) {
        SDL_Surface* image = SDL_LoadBMP(argv[f]);
        if(image == NULL) {
            fprintf(stderr, "Could not load %s: %s\n", argv[f], SDL_GetError());
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

        int bytes = 0;

        SDL_LockSurface(data);
        switch(filetype) {
            case monochrome: {
                    int i;
                    for(i = 0; i != data->w * data->h; i+=8) {
                        int x = 0;
                        int j;
                        for(j = 0; j != 8; ++j) {
                            x |= ((Uint8 *)data->pixels)[i+j] << (8-j);
                        }
                        print_byte(output, &bytes, x);
                    }
                }
                break;
            case grayscale: {
                    int i;
                    for(i = 0; i != data->w * data->h; ++i) {
                        int x = ((Uint8 *)data->pixels)[i];
                        print_byte(output, &bytes, x);
                    }
                }
                break;
            default:
                abort();
                break;
        }
        SDL_UnlockSurface(data);

        printf("%s ", argv[f]);
        printf("(%s, ", filetype == monochrome ? "monochrome" : "grayscale");
        printf("%ix%i pixels, ", data->w, data->h);
        printf("%i bytes)\n", bytes);
        total += bytes;

        SDL_FreeSurface(data);

        if(f != argc-2) {
            fprintf(output,",\n");
        }
    }

    fprintf(output, ";\n");
    fclose(output);

    printf("%i total bytes\n", total);

    SDL_Quit();
    return 0;
}
