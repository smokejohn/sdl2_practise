#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// a circle structure
struct Circle {
    int x, y;
    int r;
};

// the dot that will move around the screen
class Dot {
   public:
    // the dimensions of the dot
    static const int DOT_WIDTH = 20;
    static const int DOT_HEIGHT = 20;

    // maximum axis velocity of the dot
    static const int DOT_VEL = 10;

    // initializes the variables
    Dot();

    // takes key presses and adjusts the dot's velocity
    void handleEvent(SDL_Event& e);

    // moves the dot
    void move();

    // shows the dot on the screen
    void render();

   private:
    // the x and y offsets of the dot
    int mPosX, mPosY;

    // the velocity of the dot
    int mVelX, mVelY;
};

// the application time based timer
class LTimer {
   public:
    // initializes variables
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
    // the click time when the timer started
    Uint32 mStartTicks;

    // the ticks stored when the timer is paused
    Uint32 mPausedTicks;

    // the timer status
    bool mPaused;
    bool mStarted;
};

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

// font
TTF_Font* gFont;

// screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int TOTAL_DATA = 10;

// data textures
LTexture gDataTextures[TOTAL_DATA];
LTexture gPromptTextTexture;

// data points
Sint32 gData[TOTAL_DATA];

bool init() {
    // initialization flag
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

    gFont = TTF_OpenFont("./resources/lazy.ttf", 28);
    if (gFont == NULL) {
        std::cout << "Unable to load lazy font! SLD_ttf Error: " << TTF_GetError() << "\n";
    }

    if (!gPromptTextTexture.loadFromRenderedText("Enter Data:", SDL_Color{0, 0, 0, 255})) {
        std::cout << "Unable to create prompt text texture!\n";
        success = false;
    }

    // open file for reading in binary
    SDL_RWops* file = SDL_RWFromFile("./resources/nums.bin", "r+b");
    // file does not exist
    if (file == NULL) {
        std::cout << "Warning: Unable to open file! SEL Error: " << SDL_GetError() << "\n";

        // create file for writing
        file = SDL_RWFromFile("./resources/nums.bin", "w+b");
        if (file != NULL) {
            std::cout << "New file created!\n";

            // init data
            for (int i = 0; i < TOTAL_DATA; ++i) {
                gData[i] = 0;
                SDL_RWwrite(file, &gData[i], sizeof(Sint32), 1);
            }

            // close file handler
            SDL_RWclose(file);
        } else {
            std::cout << "Error: Unable to create file! SDL Error: " << SDL_GetError() << "\n";
            success = false;
        }
    }
    // file exists
    else {
        // load data
        std::cout << "Reading file...!\n";
        for (int i = 0; i < TOTAL_DATA; ++i) {
            SDL_RWread(file, &gData[i], sizeof(Sint32), 1);
        }

        // close file handler
        SDL_RWclose(file);
    }

    // initialize data textures
    gDataTextures[0].loadFromRenderedText(std::to_string(gData[0]), SDL_Color{255,0,0,255});
    for (int i = 1; i < TOTAL_DATA; ++i ) {
        gDataTextures[i].loadFromRenderedText(std::to_string(gData[i]), SDL_Color{0,0,0,255});
    }
    return success;
}

void close() {
    // open data for writing
    SDL_RWops* file = SDL_RWFromFile("./resources/nums.bin", "w+b");
    if (file != NULL) {
        // save data
        for (int i = 0; i < TOTAL_DATA; ++i) {
            SDL_RWwrite(file, &gData[i], sizeof(Sint32), 1);
        }

        // close file handler
        SDL_RWclose(file);
    } else {
        std::cout << "Error: Unable to save file! SDL Error: " << SDL_GetError() << "\n";
    }

    // free data textures
    for (int i = 0; i < TOTAL_DATA; ++i) {
        gDataTextures[i].free();
    }

    gPromptTextTexture.free();

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
            SDL_Color highlightColor = {255, 0, 0, 255};

            // the current input point
            int currentData = 0;

            // while application is running
            while (!quit) {
                // the rerender text flag
                bool renderText = false;

                // handle events on queue
                while (SDL_PollEvent(&e) != 0) {
                    // user requests quit
                    if (e.type == SDL_QUIT) {
                        quit = true;
                    }

                    // special key input
                    else if (e.type == SDL_KEYDOWN) {
                        switch (e.key.keysym.sym) {
                            // previous data entry
                            case SDLK_UP:
                                // rerender previous entry point
                                gDataTextures[currentData].loadFromRenderedText(std::to_string(gData[currentData]),
                                                                                textColor);
                                --currentData;
                                if (currentData < 0) {
                                    currentData = TOTAL_DATA - 1;
                                }
                                // rerender current entry input point
                                gDataTextures[currentData].loadFromRenderedText(std::to_string(gData[currentData]),
                                                                                highlightColor);
                                break;

                            // next data entry
                            case SDLK_DOWN:
                                // rerender previous entry input point
                                gDataTextures[currentData].loadFromRenderedText(std::to_string(gData[currentData]),
                                                                                textColor);
                                ++currentData;
                                if (currentData == TOTAL_DATA) {
                                    currentData = 0;
                                }
                                // rerender current entry input point
                                gDataTextures[currentData].loadFromRenderedText(std::to_string(gData[currentData]),
                                                                                highlightColor);
                                break;

                            // decrement input point
                            case SDLK_LEFT:
                                --gData[currentData];
                                gDataTextures[currentData].loadFromRenderedText(std::to_string(gData[currentData]),
                                                                                highlightColor);
                                break;

                            // increment input point
                            case SDLK_RIGHT:
                                ++gData[currentData];
                                gDataTextures[currentData].loadFromRenderedText(std::to_string(gData[currentData]),
                                                                                highlightColor);
                                break;
                        }
                    }
                }

                // clear screen
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                SDL_RenderClear(gRenderer);

                // render text textures
                gPromptTextTexture.render((SCREEN_WIDTH - gPromptTextTexture.getWidth()) / 2, 0);

                for (int i = 0; i < TOTAL_DATA; ++i) {
                    gDataTextures[i].render((SCREEN_WIDTH - gDataTextures[i].getWidth()) / 2,
                                            gPromptTextTexture.getHeight() + gDataTextures[0].getHeight() * i);
                }

                // update the screen
                SDL_RenderPresent(gRenderer);
            }
            // disable text input
            SDL_StopTextInput();
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
LTimer::LTimer() {
    // initialize the variables
    mStartTicks = 0;
    mPausedTicks = 0;

    mPaused = false;
    mStarted = false;
}

void LTimer::start() {
    // start the timer
    mStarted = true;

    // unpause the timer
    mPaused = false;

    // get the current clock time
    mStartTicks = SDL_GetTicks();
    mPausedTicks = 0;
}

void LTimer::stop() {
    // stop the timer
    mStarted = false;

    // unpause the timer
    mPaused = false;

    // clear tick variables
    mStartTicks = 0;
    mPausedTicks = 0;
}

void LTimer::pause() {
    // if the timer is running and isn't already paused
    if (mStarted && !mPaused) {
        // pause the timer
        mPaused = true;

        // calculate the pased ticks
        mPausedTicks = SDL_GetTicks() - mStartTicks;
        mStartTicks = 0;
    }
}

void LTimer::unpause() {
    // if the timer is running and paused
    if (mStarted && mPaused) {
        // unpause the timer
        mPaused = false;

        // reset the starting ticks
        mStartTicks = SDL_GetTicks() - mPausedTicks;

        // reset the paused ticks
        mPausedTicks = 0;
    }
}

Uint32 LTimer::getTicks() {
    // the actual timer time
    Uint32 time = 0;

    // if the timer is running
    if (mStarted) {
        // if the timer is paused
        if (mPaused) {
            // return the number of ticks when the timer was paused
            time = mPausedTicks;
        } else {
            // return the current time minus the start time
            time = SDL_GetTicks() - mStartTicks;
        }
    }

    return time;
}

bool LTimer::isStarted() { return mStarted; }

bool LTimer::isPaused() { return mPaused; }

Dot::Dot() {
    // initialize the offsets
    mPosX = 0;
    mPosY = 0;

    // initialize the velocity
    mVelX = 0;
    mVelY = 0;
}

void Dot::handleEvent(SDL_Event& e) {
    // if a key was pressed
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        // adjust the velocity
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
    }

    // if a key was released
    else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
        // adjust the velocity
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

void Dot::move() {
    // move the dot left or right
    mPosX += mVelX;

    // if the dot went too far to the left or right
    if ((mPosX < 0) || (mPosX + DOT_WIDTH > SCREEN_WIDTH)) {
        // move back
        mPosX -= mVelX;
    }

    mPosY += mVelY;

    // if the dot went too far up or down
    if ((mPosY < 0) || (mPosY + DOT_HEIGHT > SCREEN_HEIGHT)) {
        // move back
        mPosY -= mVelY;
    }
}

