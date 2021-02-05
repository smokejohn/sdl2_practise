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

    // creates blank texture
    bool createBlank(int width, int height);

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
    void copyPixels(void* pixels);
    int getPitch();
    Uint32 getPixel32(unsigned int x, unsigned int y);

   private:
    // the actual hardware texture
    SDL_Texture* mTexture;
    void* mPixels;
    int mPitch;

    // image dimensions
    int mWidth;
    int mHeight;
};

// a test animation stream
class DataStream {
   public:
    // initializes internals
    DataStream();

    // loads initial data
    bool loadMedia();

    // deallocator
    void free();

    // gets current frame data
    void* getBuffer();

   private:
    // internal data
    SDL_Surface* mImages[4];
    int mCurrentImage;
    int mDelayFrames;
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
LTexture gStreamingTexture;

// animation stream
DataStream gDataStream;

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

    // load blank texture
    if (!gStreamingTexture.createBlank(64, 205)) {
        std::cout << "Failed to creat streaming texture!\n";
        success = false;
    }

    // load data stream
    if (!gDataStream.loadMedia()) {
        std::cout << "Unable to load data stream!\n";
        success = false;
    }

    return success;
}

void close() {
    // destroy data
    gStreamingTexture.free();
    gDataStream.free();

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

                // copy frame from buffer
                gStreamingTexture.lockTexture();
                gStreamingTexture.copyPixels(gDataStream.getBuffer());
                gStreamingTexture.unlockTexture();

                // render frame
                gStreamingTexture.render((SCREEN_WIDTH - gStreamingTexture.getWidth()) / 2,
                                         (SCREEN_HEIGHT - gStreamingTexture.getHeight()) / 2);

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
                // enable blending on texture
                SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_BLEND);

                // lock texture for manupulation
                SDL_LockTexture(newTexture, NULL, &mPixels, &mPitch);

                // copy loaded/formatted surface pixels
                memcpy(mPixels, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h);

                // get image dimensions
                mWidth = formattedSurface->w;
                mHeight = formattedSurface->h;

                // get pixel data in editable format
                Uint32* pixels = (Uint32*)mPixels;
                int pixelCount = (mPitch / 4) * mHeight;

                // map colors
                Uint32 colorKey = SDL_MapRGB(formattedSurface->format, 0, 255, 255);
                Uint32 transparent = SDL_MapRGBA(formattedSurface->format, 255, 255, 255, 0);

                // color key pixels
                for (int i = 0; i < pixelCount; ++i) {
                    if (pixels[i] == colorKey) {
                        pixels[i] = transparent;
                    }
                }

                // unlock texture to update
                SDL_UnlockTexture(newTexture);
                mPixels = NULL;
            }

            // get rid of old formatted surface
            SDL_FreeSurface(formattedSurface);
        }

        // get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }

    // return success
    mTexture = newTexture;
    return mTexture != NULL;
}

Uint32 LTexture::getPixel32(unsigned int x, unsigned int y) {
    // convert the pixels to 32 bit
    Uint32* pixels = (Uint32*)mPixels;

    // get the pixel requested
    return pixels[(y * (mPitch / 4)) + x];
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
        mPixels = NULL;
        mPitch = 0;
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

bool LTexture::createBlank(int width, int height) {
    // create uninitialized texture
    mTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (mTexture == NULL) {
        std::cout << "Unable to create blank texture! SDL Error: " << SDL_GetError() << "\n";
    } else {
        mWidth = width;
        mHeight = height;
    }

    return mTexture != NULL;
}

void LTexture::copyPixels(void* pixels) {
    // texture is locked
    if (mPixels != NULL) {
        // copy the locked pixels
        memcpy(mPixels, pixels, mPitch * mHeight);
    }
}

DataStream::DataStream() {
    mImages[0] = NULL;
    mImages[1] = NULL;
    mImages[2] = NULL;
    mImages[3] = NULL;

    mCurrentImage = 0;
    mDelayFrames = 4;
}

bool DataStream::loadMedia() {
    bool success = true;

    for (int i = 0; i < 4; ++i) {
        char path[64] = "";
        sprintf(path, "./resources/images/foo_walk_%d.png", i);

        SDL_Surface* loadedSurface = IMG_Load(path);
        if (loadedSurface == NULL) {
            std::cout << "Unable to load " << path << "! SDL_image error: " << IMG_GetError() << "\n";
            success = false;
        } else {
            mImages[i] = SDL_ConvertSurfaceFormat(loadedSurface, SDL_PIXELFORMAT_RGBA8888, 0);
        }

        SDL_FreeSurface(loadedSurface);
    }

    return success;
}

void DataStream::free() {
    for (int i = 0; i < 4; ++i) {
        SDL_FreeSurface(mImages[i]);
    }
}

void* DataStream::getBuffer() {
    --mDelayFrames;

    if (mDelayFrames == 0) {
        ++mCurrentImage;
        mDelayFrames = 4;
    }

    if (mCurrentImage == 4) {
        mCurrentImage = 0;
    }

    return mImages[mCurrentImage]->pixels;
}
