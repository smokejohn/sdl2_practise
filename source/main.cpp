#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_render.h>
#include <SDL_ttf.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// texture wrapper class
class LTexture {
   public:
    // initializes variables
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

    // pixel manipulators
    bool lockTexture();
    bool unlockTexture();
    void* getPixels();
    int getPitch();

   private:
    // the actual hardware texture
    SDL_Texture* mTexture;
    void* mPixels;
    int mPitch;

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

// the window
SDL_Window* gWindow = NULL;

// the window renderer
SDL_Renderer* gRenderer = NULL;

// scene textures
LTexture gFooTexture;

// font
TTF_Font* gFont;

bool init() {
    // initialization flag
    bool success = true;

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        success = false;
    } else {
        // set texture filtering to linear
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
            std::cout << "Warning: Linear texture filtering not enabled!\n";
        }

        // create window
        gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                   SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
        if (gWindow == NULL) {
            std::cout << "Window could not be created!\n";
            success = false;
        } else {
            // create renderer for window
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (gRenderer == NULL) {
                std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << "\n";
                success = false;
            } else {
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

    // load foo texture
    if (!gFooTexture.loadFromFile("./resources/images/foo.png")) {
        std::cout << "Failed to load foo texture!\n";
        success = false;
    } else {
        // lock texture
        if (!gFooTexture.lockTexture()) {
            std::cout << "Unable to lock Foo texture!\n";
        } 
        else {
            // manual color key
            // allocate format from window
            Uint32 format = SDL_GetWindowPixelFormat(gWindow);
            SDL_PixelFormat* mappingFormat = SDL_AllocFormat(format);

            // get pixel data
            Uint32* pixels = (Uint32*)gFooTexture.getPixels();
            int pixelCount = (gFooTexture.getPitch() / 4) * gFooTexture.getHeight();

            // map colors
            Uint32 colorKey = SDL_MapRGB(mappingFormat, 0, 255, 255);
            Uint32 transparent = SDL_MapRGBA(mappingFormat, 255, 255, 255, 0);

            // color key pixels
            for (int i = 0; i < pixelCount; ++i) {
                if (pixels[i] == colorKey) {
                    pixels[i] = transparent;
                }
            }

            // unlock texture
            gFooTexture.unlockTexture();

            // free format
            SDL_FreeFormat(mappingFormat);
        }
    }

    return success;
}

void close() {
    // destroy data
    gFooTexture.free();

    // destroy windows
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gRenderer = NULL;
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
        // the level tiles

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

                    if (e.type == SDL_KEYDOWN) {
                        if (e.key.keysym.sym == SDLK_ESCAPE) {
                            quit = true;
                        }
                    }
                }

                // clear screen
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                SDL_RenderClear(gRenderer);

                gFooTexture.render((SCREEN_WIDTH - gFooTexture.getWidth()) / 2,
                                   (SCREEN_HEIGHT - gFooTexture.getHeight()) / 2);

                // update the screen
                SDL_RenderPresent(gRenderer);
            }
        }
        // free resources and close SDL
        close();
    }

    return 0;
}

LTexture::LTexture() {
    // initialize
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
    mPixels = NULL;
    mPitch = 0;
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
        // convert surface to display format
        SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(loadedSurface, SDL_GetWindowPixelFormat(gWindow), 0);
        if (formattedSurface == NULL) {
            std::cout << "Unable to convert loaded surface to display format! SDL Error: " << SDL_GetError() << "\n";
        } else {
            // create blank streamable texture
            newTexture = SDL_CreateTexture(gRenderer, SDL_GetWindowPixelFormat(gWindow), SDL_TEXTUREACCESS_STREAMING,
                                           formattedSurface->w, formattedSurface->h);
            if (newTexture == NULL) {
                std::cout << "Unable to create blank texture! SDL Error: " << SDL_GetError() << "\n";
            } else {
                // lock texture for manupulation
                SDL_LockTexture(newTexture, NULL, &mPixels, &mPitch);

                // copy loaded/formatted surface pixels
                memcpy(mPixels, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h);

                // unlock texture to update
                SDL_UnlockTexture(newTexture);
                mPixels = NULL;

                // get image dimensions
                mWidth = formattedSurface->w;
                mHeight = formattedSurface->h;
            }

            // get rid of old formatted surface
            SDL_FreeSurface(formattedSurface);
        }
    }

    // return success
    mTexture = newTexture;
    return mTexture != NULL;
}

bool LTexture::lockTexture() {
    bool success = true;

    // texture is already locked
    if (mPixels != NULL) {
        std::cout << "Texture is already locked!\n";
        success = false;
    } else {
        // lock texture
        if (SDL_LockTexture(mTexture, NULL, &mPixels, &mPitch) != 0) {
            std::cout << "Unable to lock texture! " << SDL_GetError() << "\n";
            success = false;
        }
    }
    return success;
}

bool LTexture::unlockTexture() {
    bool success = true;

    // texture is already unlocked
    if (mPixels == NULL) {
        std::cout << "Texture is not locked!\n";
        success = false;
    } else {
        // unlock texture
        SDL_UnlockTexture(mTexture);
        mPixels = NULL;
        mPitch = 0;
    }
    return success;
}

void* LTexture::getPixels() { return mPixels; }

int LTexture::getPitch() { return mPitch; }

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
