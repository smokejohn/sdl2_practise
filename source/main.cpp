#include <iostream>
#include <SDL.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char *argv[])
{
    std::cout << "starting sdltest\n";

    // the window we will be rendering to
    SDL_Window* window = NULL;

    // the surface contained by the window
    SDL_Surface* screenSurface = NULL;

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
    }
    else
    {
        // create window
        window = SDL_CreateWindow( "SDL Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL)
        {
            std::cout << "Window could not be created! " << SDL_GetError() << "\n";
        }
        else
        {
            // get window surface
            screenSurface = SDL_GetWindowSurface(window);

            // fill the surface white
            SDL_FillRect(screenSurface, NULL, SDL_MapRGB( screenSurface->format, 255, 255, 255));

            // update the surface
            SDL_UpdateWindowSurface(window);

            // wait two seconds
            SDL_Delay(2000);
        }
    }

    // destroy window
    SDL_DestroyWindow(window);

    // quit SDL subsystems
    SDL_Quit();

    return 0;
}
