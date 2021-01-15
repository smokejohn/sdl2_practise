#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <iostream>
#include <sstream>
#include <string>

// texture wrapper class
class LTexture {
   public:
    // initalizes variables
    LTexture();

    // deallocates memory
    ~LTexture();

    // loads image at specified path
    bool loadFromFile(std::string path);

#if defined(SDL_TTF_MAJOR_VERSION)
    // creates image from font string
    bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
#endif

    // deallocates texture
    void free();

    // set color modulation
    void setColor(Uint8 red, Uint8 green, Uint8 blue);

    // set blending
    void setBlendMode(SDL_BlendMode blending);

    // set alpha modulation
    void setAlpha(Uint8 alpha);

    // renders texture at given point
    void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL,
                SDL_RendererFlip flip = SDL_FLIP_NONE);

    // gets image dimensions
    int getWidth();
    int getHeight();

   private:
    // the actual hardware texture
    SDL_Texture* mTexture;

    // image dimensions
    int mWidth;
    int mHeight;
};

// starts up SDL and creates a window
bool init();
// loads media
bool loadMedia();
// frees media and shuts down SDL
void close();

// loads individual image as texture
SDL_Texture* loadTexture(std::string path);

// the window we will be rendering to
SDL_Window* gWindow = NULL;

// the window renderer
SDL_Renderer* gRenderer = NULL;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// font
TTF_Font* gFont;

// prompt texture
LTexture gPromptTexture;
LTexture gTimeTextTexture;

LTexture::LTexture() {
    // initialize
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
}

LTexture::~LTexture() {
    // deallocate
    free();
}

bool LTexture::loadFromFile(std::string path) {
    // get rid of preexisting texture
    free();

    // the final texture
    SDL_Texture* newTexture = NULL;

    // load image at specified path
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == NULL) {
        std::cout << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << "\n";
    } else {
        // color key the image
        SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 255, 255));

        // create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == NULL) {
            std::cout << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << "\n";
        } else {
            // get image dimensions
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }

        // get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }

    // return success
    mTexture = newTexture;
    return mTexture != NULL;
}

#if defined(SDL_TTF_MAJOR_VERSION)
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor) {
    // get rid of preexisting texture
    free();

    // render text surface
    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
    if (textSurface == NULL) {
        std::cout << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << "\n";
    } else {
        // create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
        if (mTexture == NULL) {
            std::cout << "Unable to create texture from renderd text! SDL Error: " << SDL_GetError() << "\n";
        } else {
            // get image dimensions
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }

        // get rid of old surface
        SDL_FreeSurface(textSurface);
    }

    // return success
    return mTexture != NULL;
}
#endif

void LTexture::free() {
    // free texture if it exists
    if (mTexture != NULL) {
        SDL_DestroyTexture(mTexture);
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
    }
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue) {
    // modulate texture
    SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending) {
    // set blending function
    SDL_SetTextureBlendMode(mTexture, blending);
}

void LTexture::setAlpha(Uint8 alpha) {
    // modulate texture alpha
    SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip) {
    // set rendering space and render to screen
    SDL_Rect renderQuad = {x, y, mWidth, mHeight};

    // set clip rendering dimensions
    if (clip != NULL) {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int LTexture::getWidth() { return mWidth; }

int LTexture::getHeight() { return mHeight; }

bool init() {
    // initalization flag
    bool success = true;

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        success = false;
    } else {
        // set texture filtering to linear
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
            std::cout << "Warning: Linear texture filtering not enabled!\n";
        }

        // create window
        gWindow = SDL_CreateWindow("SDL Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                   SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL) {
            std::cout << "Window could not be created! " << SDL_GetError() << "\n";
            success = false;
        } else {
            // create vsynced renderer for window
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (gRenderer == NULL) {
                std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << "\n";
                success = false;
            } else {
                // initialize renderer color

                // initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if (!(IMG_Init(imgFlags) & imgFlags)) {
                    std::cout << "SDL_image could not initialize! SDL_image Eror: " << IMG_GetError() << "\n";
                    success = false;
                }

                // initialize SDL_mixer
                if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
                    std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << "\n";
                    success = false;
                }

                // initialize SDL_ttf
                if (TTF_Init() == -1) {
                    std::cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << "\n";
                    success = false;
                }
            }
        }
    }

    return success;
}

bool loadMedia() {
    // loading success flag
    bool success = true;

    // open the font
    gFont = TTF_OpenFont("./resources/lazy.ttf", 28);
    if (gFont == NULL) {
        std::cout << "Failed to load lazy font! SDL_ttf Error: " << TTF_GetError() << "\n";
        success = false;
    } else {
        // set text color as black
        SDL_Color textColor = {0, 0, 0, 255};

        // load prompt texture
        if (!gPromptTexture.loadFromRenderedText("Press Enter to Reset Start Time.", textColor)) {
            std::cout << "Unable to render prompt texture!\n";
            success = false;
        }
    }

    return success;
}

void close() {
    // free loaded images
    gPromptTexture.free();
    gTimeTextTexture.free();

    // destroy window
    SDL_DestroyRenderer(gRenderer);
    gRenderer = NULL;
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    // quit SDL subsystems
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
    TTF_Quit();
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

            // set text color as black
            SDL_Color textColor = {0, 0, 0, 255};

            // current time start time
            Uint32 startTime = 0;

            // in memory text stream
            std::stringstream timeText;

            // while application is running
            while (!quit) {
                // handle events on queue
                while (SDL_PollEvent(&e) != 0) {
                    // user requests quit
                    if (e.type == SDL_QUIT) {
                        quit = true;
                    }

                    // reset start time on return keypress
                    else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
                        startTime = SDL_GetTicks();
                    }
                }
                // set text to be rendered
                timeText.str("");
                timeText << "Milliseconds since start time " << SDL_GetTicks() - startTime;

                // render text
                if (!gTimeTextTexture.loadFromRenderedText(timeText.str().c_str(), textColor)) {
                    std::cout << "Unable to render time texture!\n";
                }

                // clear screen
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                SDL_RenderClear(gRenderer);

                gPromptTexture.render((SCREEN_WIDTH - gPromptTexture.getWidth()) / 2, 0);
                gTimeTextTexture.render((SCREEN_WIDTH - gTimeTextTexture.getWidth()) / 2,
                                        (SCREEN_HEIGHT - gPromptTexture.getHeight()) / 2);

                // update the screen
                SDL_RenderPresent(gRenderer);
            }
        }
    }

    // free resources and close SDL
    close();

    return 0;
}

SDL_Texture* loadTexture(std::string path) {
    // the final texture
    SDL_Texture* newTexture = NULL;

    // load image at specified path
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == NULL) {
        std::cout << "Unable to load image " << path.c_str() << "! SDL_image Error: " << IMG_GetError() << "\n";
    } else {
        // create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == NULL) {
            std::cout << "Unable to create texture from " << path.c_str() << "! SDL Error: " << SDL_GetError() << "\n";
        }

        // get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }

    return newTexture;
}

