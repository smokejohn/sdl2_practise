#include <SDL.h>

#include <iostream>
//

// starts up SDL and creates a window
bool init();
// loads media
bool loadMedia();
// frees media and shuts down SDL
void close();

SDL_Surface* loadSurface(std::string path);

// the window we will be rendering to
SDL_Window* gWindow = NULL;

// the surface contained by the window
SDL_Surface* gScreenSurface = NULL;

// current displayed image
SDL_Surface* gStretchedSurface = NULL;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Surface* loadSurface(std::string path) {
    // the final optimized image
    SDL_Surface* optimizedSurface = NULL;

    // load image at specified path
    SDL_Surface* loadedSurface = SDL_LoadBMP(path.c_str());
    if (loadedSurface == NULL) {
        std::cout << "Unable to load image " << path << "! SDL Error: " << SDL_GetError() << "\n";
    }
    else {
        // convert surface to screen format
        optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);
        if (optimizedSurface == NULL) {
            std::cout << "Unable to optimize image " << path << "! SDL Error: " << SDL_GetError() << "\n";
        }

        // get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }

    return optimizedSurface;
}

bool init() {
    // initalization flag
    bool success = true;

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
    } else {
        // create window
        gWindow = SDL_CreateWindow("SDL Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                   SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL) {
            std::cout << "Window could not be created! " << SDL_GetError() << "\n";
        } else {
            // get window surface
            gScreenSurface = SDL_GetWindowSurface(gWindow);
        }
    }

    return success;
}

bool loadMedia() {
    // loading success flag
    bool success = true;

    gStretchedSurface = loadSurface("./resources/stretch.bmp");
    if (gStretchedSurface == NULL) {
        std::cout << "Media could not be loaded! SDL_ERROR: " << SDL_GetError() << "\n";
        success = false;
    }

    return success;
}

void close() {
    // destroy surface
    SDL_FreeSurface(gStretchedSurface);
    gStretchedSurface = NULL;

    // destroy window
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    // quit SDL subsystems
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    std::cout << "starting sdltest\n";
    if (!init()) {
        std::cout << "Failed to initialize!\n";
    } else {
        // load media
        if (!loadMedia()) {
            std::cout << "Failed to load media!\n";
        } else {
            // main loop flag
            bool quit = false;

            // event handler
            SDL_Event e;

            // while application is running
            while (!quit) {
                // handle events on queue
                while (SDL_PollEvent(&e) != 0) {
                    // user requests quit
                    if (e.type == SDL_QUIT) {
                        quit = true;
                    }
                }

                // apply the image stretched
                SDL_Rect stretchRect;
                stretchRect.x = 0;
                stretchRect.y = 0;
                stretchRect.w = SCREEN_WIDTH;
                stretchRect.h = SCREEN_HEIGHT;
                SDL_BlitScaled(gStretchedSurface, NULL, gScreenSurface, &stretchRect);

                // update the surface
                SDL_UpdateWindowSurface(gWindow);
            }
        }
    }

    // free resources and close SDL
    close();

    return 0;
}
