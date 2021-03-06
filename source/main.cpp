#include <SDL.h>
#include <SDL_image.h>
#include <SDL_thread.h>
#include <SDL_mixer.h>
#include <SDL_render.h>
#include <SDL_ttf.h>

#include <iostream>
#include <string>

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
    bool createBlank(int width, int height, SDL_TextureAccess = SDL_TEXTUREACCESS_STREAMING);

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

    // set self as render target
    void setAsRenderTarget();

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

// the application time based timer
class LTimer {
   public:
    LTimer();

    // the various clock actions
    void start();
    void stop();
    void pause();
    void unpause();

    // gets the timer's time
    Uint32 getTicks();

    // checks the status of the timer
    bool isStarted();
    bool isPaused();

   private:
    // the ticks stored when the timer is running
    Uint32 mStartTicks;

    // the ticks stored when the timer is paused
    Uint32 mPausedTicks;

    // the timer status
    bool mPaused;
    bool mStarted;
};

// the dot that will move around the screen
class Dot {
   public:
    // the dimensions of the dot
    static const int DOT_WIDTH = 20;
    static const int DOT_HEIGHT = 20;

    // maximum axis velocity of the dot
    static const int DOT_VEL = 640;

    // initializes variables
    Dot();

    // takes key presses and adjusts the dot's velocity
    void handleEvent(SDL_Event& e);

    // moves the dot
    void move(float timeStep);

    // renders the dot to the screen
    void render();

   private:
    float mPosX, mPosY;
    float mVelX, mVelY;
};

// our owrker functions
int producer(void* data);
int consumer(void* data);
void produce();
void consume();

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
LTexture gSplashTexture;

// the protective mutex (mutually exclusive)
SDL_mutex* gBufferLock = NULL;

// the conditions
SDL_cond* gCanProduce = NULL;
SDL_cond* gCanConsume = NULL;

// the "data buffer"
int gData = -1;

// font
TTF_Font* gFont;

int producer(void* data) {
    std::cout << "\nProducer started...\n";

    // seed thread random
    srand(SDL_GetTicks());

    // produce
    for(int i = 0; i < 5; ++i) {
        // wait
        SDL_Delay(rand() % 1000);

        // produce
        produce();
    }

    std::cout << "\nProducer finisehd!\n";

    return 0;
}

int consumer(void* data) {
    std::cout << "\nConsumer started...\n";

    // seed thread random
    srand(SDL_GetTicks());

    // consume
    for (int i = 0; i < 5; ++i) {
        // wait
        SDL_Delay(rand() % 1000);

        // consume
        consume();
    }

    std::cout << "\nConsumer finished...\n";

    return 0;
}

void produce() {
    // lock
    SDL_LockMutex(gBufferLock);

    // if the buffer is full
    if (gData != -1) {
        // wait for buffer to be cleared
        std::cout << "\nProducer encountered full buffer, waiting for consumer to empty buffer...\n";

        SDL_CondWait(gCanProduce, gBufferLock);
    }

    // fill and show buffer
    gData = rand() % 255;
    std::cout << "\nProduced " << gData << std::endl;

    // unlock
    SDL_UnlockMutex(gBufferLock);

    // signal consumer
    SDL_CondSignal(gCanConsume);
}

void consume() {
    // lock
    SDL_LockMutex(gBufferLock);

    // if the buffer is empty
    if (gData == -1) {
        // wait for the buffer to be filled
        std::cout << "\nConsumer encountered empty buffer, waiting or producer to fill buffer...\n";
        SDL_CondWait(gCanConsume, gBufferLock);
    }

    // show and empty buffer
    std::cout << "\nConsumed " << gData << std::endl;
    gData = -1;

    // unlock
    SDL_UnlockMutex(gBufferLock);

    // signal producer
    SDL_CondSignal(gCanProduce);
}

LTimer::LTimer() {
    mPausedTicks = 0;
    mStartTicks = 0;

    mPaused = false;
    mStarted = false;
}


void LTimer::start() {
    mStarted = true;
    mPaused = false;

    mStartTicks = SDL_GetTicks();
    mPausedTicks = 0;
}

void LTimer::stop() {
    mStarted = false;
    mPaused = false;

    mStartTicks = 0;
    mPausedTicks = 0;
}

void LTimer::pause() {
    if (mStarted && !mPaused) {
        mPaused = true;

        mPausedTicks = SDL_GetTicks() - mStartTicks;
        mStartTicks = 0;
    }
}

void LTimer::unpause() {
    if (mStarted && mPaused) {
        mPaused = false;

        mStartTicks = SDL_GetTicks() - mPausedTicks;
        mPausedTicks = 0;
    }
}

Uint32 LTimer::getTicks() {
    // the actual time
    Uint32 time = 0;

    if (mStarted) {
        if (mPaused) {
            time = mPausedTicks;
        } else {
            time = SDL_GetTicks() - mStartTicks;
        }
    }
    return time;
}

bool LTimer::isStarted() { return mStarted; }

bool LTimer::isPaused() { return mStarted && mPaused; }

Dot::Dot() {
    mPosX = 0.f;
    mPosY = 0.f;
    mVelX = 0.f;
    mVelY = 0.f;
}

void Dot::move(float timeStep) {
    // move the dot left or right
    mPosX += mVelX * timeStep;

    // if the dot went too far to the left or right
    if (mPosX < 0) {
        mPosX = 0;
    } else if (mPosX > SCREEN_WIDTH - DOT_WIDTH) {
        mPosX = SCREEN_WIDTH - DOT_WIDTH;
    }

    mPosY += mVelY * timeStep;

    // if the dot went too far up or down
    if (mPosY < 0) {
        mPosY = 0;
    } else if (mPosY > SCREEN_HEIGHT - DOT_HEIGHT) {
        mPosY = SCREEN_HEIGHT - DOT_HEIGHT;
    }
}

void Dot::render() {
    // show the dot
    gSplashTexture.render((int)mPosX, (int)mPosY);
}

void Dot::handleEvent(SDL_Event& e) {
    // add velocity on press
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                mVelY -= DOT_VEL;
                break;
            case SDLK_DOWN:
                mVelY += DOT_VEL;
                break;
            case SDLK_LEFT:
                mVelX -= DOT_VEL;
                break;
            case SDLK_RIGHT:
                mVelX += DOT_VEL;
                break;
        }
    } else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                mVelY += DOT_VEL;
                break;
            case SDLK_DOWN:
                mVelY -= DOT_VEL;
                break;
            case SDLK_LEFT:
                mVelX += DOT_VEL;
                break;
            case SDLK_RIGHT:
                mVelX -= DOT_VEL;
                break;
        }
    }
}

bool init() {
    // initialization flag
    bool success = true;

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
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
    // create the mutex
    gBufferLock = SDL_CreateMutex();

    // create conditions
    gCanProduce = SDL_CreateCond();
    gCanConsume = SDL_CreateCond();

    // loading success flag
    bool success = true;

    // load dot texture
    if (!gSplashTexture.loadFromFile("./resources/images/splash_thread.png")) {
        std::cout << "Unable to load splash texture! SDL Error: " << SDL_GetError() << "\n";
        success = false;
    }

    return success;
}

void close() {
    // destroy data
    gSplashTexture.free();

    // destroy the mutex
    SDL_DestroyMutex(gBufferLock);
    gBufferLock = NULL;

    // destroy the conditions
    SDL_DestroyCond(gCanConsume);
    SDL_DestroyCond(gCanProduce);
    gCanConsume = NULL;
    gCanProduce = NULL;

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
        // load media
        if (!loadMedia()) {
            std::cout << "Failed to load media!\n";
        } else {
            // main loop flag
            bool quit = false;

            // event handler
            SDL_Event e;

            // run the threads
            srand(SDL_GetTicks());
            SDL_Thread* threadA = SDL_CreateThread(producer, "Thread A", (void*)"Thread A");
            SDL_Delay(16 + rand() % 32);
            SDL_Thread* threadB = SDL_CreateThread(consumer, "Thread B", (void*)"Thread B");

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

                // render splash
                gSplashTexture.render(0,0);

                // update the screen
                SDL_RenderPresent(gRenderer);
            }

            // wait for threads to finish
            SDL_WaitThread(threadA, NULL);
            SDL_WaitThread(threadB, NULL);
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

void LTexture::copyPixels(void* pixels) {
    // texture is locked
    if (mPixels != NULL) {
        // copy the locked pixels
        memcpy(mPixels, pixels, mPitch * mHeight);
    }
}

bool LTexture::createBlank(int width, int height, SDL_TextureAccess access) {
    // create uninitialized texture
    mTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, access, width, height);
    if (mTexture == NULL) {
        std::cout << "Unable to create blank texture! SDL Error: " << SDL_GetError() << "\n";
    } else {
        mWidth = width;
        mHeight = height;
    }

    return mTexture != NULL;
}

void LTexture::setAsRenderTarget() {
    // make self render target
    SDL_SetRenderTarget(gRenderer, mTexture);
}
