#include <iostream>
#include <SDL.h>

// starts up SDL and creates a window
bool init();

// loads media
bool loadMedia();

// frees media and shuts down SDL
void close();

// the window we will be rendering to
SDL_Window* gWindow = NULL;

// the surface contained by the window
SDL_Surface* gScreenSurface = NULL;

// the image we will load and show on the screen
SDL_Surface* gXOut = NULL;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

bool init()
{
    // initalization flag
    bool success = true;

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
    }
    else
    {
        // create window
        gWindow = SDL_CreateWindow( "SDL Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            std::cout << "Window could not be created! " << SDL_GetError() << "\n";
        }
        else
        {
            // get window surface
            gScreenSurface = SDL_GetWindowSurface(gWindow);
        }
    }

    return success;
}

bool loadMedia()
{
    // loading success flag
    bool success = true;

    // load splash image
    gXOut = SDL_LoadBMP("./resources/x.bmp");

    if (gXOut == NULL)
    {
        std::cout << "Unable to load image " << "x.bmp" << "! SDL Error: " << SDL_GetError() << "\n";
        success = false;
    }

    return success;
}

void close()
{
    // deallocate surface
    SDL_FreeSurface(gXOut);
    gXOut = NULL;

    // destroy window
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    // quit SDL subsystems
    SDL_Quit();
}


int main(int argc, char *argv[])
{
    std::cout << "starting sdltest\n";
    if (!init())
    {
        std::cout << "Failed to initialize!\n";
    }
    else
    {
        // load media
        if( !loadMedia())
        {
            std::cout << "Failed to load media!\n";
        }
        else
        {
            // main loop flag
            bool quit = false;

            // event handler
            SDL_Event e;

            // while application is running
            while (!quit)
            {
                // handle events on queue
                while (SDL_PollEvent( &e) != 0)
                {
                    // user requests quit
                    if (e.type == SDL_QUIT)
                    {
                        quit = true;
                    }
                }

            // apply the image
            SDL_BlitSurface( gXOut, NULL, gScreenSurface, NULL);

            // update the surface
            SDL_UpdateWindowSurface(gWindow);
            }

        }
    }

    // free resources and close SDL
    close();

    return 0;
}
