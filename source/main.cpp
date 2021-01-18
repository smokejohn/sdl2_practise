#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// the dot that will move around the screen
class Dot {
   public:
    // the dimensions of the dot
    static const int DOT_WIDTH = 20;
    static const int DOT_HEIGHT = 20;

    // maximum axis velocity of the dot
    static const int DOT_VEL = 1;

    // initalizes the variables
    Dot(int x, int y);

    // takes key presses and adjusts the dot's velocity
    void handleEvent(SDL_Event& e);

    // moves the dot
    void move(std::vector<SDL_Rect>& otherColliders);

    // shows the dot on the screen
    void render();

    // gets the collision boxes
    std::vector<SDL_Rect>& getColliders();

   private:
    // the x and y offsets of the dot
    int mPosX, mPosY;

    // the velocity of the dot
    int mVelX, mVelY;

    // dot's collision boxes
    std::vector<SDL_Rect> mColliders;

    // moves the collision boxes relative to the dot's offset
    void shiftColliders();
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

// box set collision detector
bool checkCollision(std::vector<SDL_Rect>& a, std::vector<SDL_Rect>& b);

// loads individual image as texture
SDL_Texture* loadTexture(std::string path);

// the window we will be rendering to
SDL_Window* gWindow = NULL;

// the window renderer
SDL_Renderer* gRenderer = NULL;

// font
TTF_Font* gFont;

// prompt texture
LTexture gDotTexture;

// screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

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

    if (!gDotTexture.loadFromFile("./resources/images/dot.bmp")) {
        std::cout << "Unable to load dot texture!\n";
        success = false;
    }

    return success;
}

void close() {
    // free loaded images
    gDotTexture.free();

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

            // the dot that will be moving around the screen
            Dot dot(0,0);

            // the dot that will be collided against
            Dot otherDot(SCREEN_WIDTH/4, SCREEN_HEIGHT/4);

            // while application is running
            while (!quit) {
                // handle events on queue
                while (SDL_PollEvent(&e) != 0) {
                    // user requests quit
                    if (e.type == SDL_QUIT) {
                        quit = true;
                    }

                    // handle input for the dot
                    dot.handleEvent(e);
                }

                // move the dot and check for collision
                dot.move(otherDot.getColliders());

                // clear screen
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                SDL_RenderClear(gRenderer);


                // render dots
                dot.render();
                otherDot.render();

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

Dot::Dot(int x, int y) {
    // initialize the offsets
    mPosX = x;
    mPosY = y;

    mColliders.resize(11);

    // initalize the velocity
    mVelX = 0;
    mVelY = 0;

    // initialize the collision boxes' width and height
    mColliders[0].w = 6;
    mColliders[0].h = 1;
    mColliders[1].w = 10;
    mColliders[1].h = 1;
    mColliders[2].w = 14;
    mColliders[2].h = 1;
    mColliders[3].w = 16;
    mColliders[3].h = 2;
    mColliders[4].w = 18;
    mColliders[4].h = 2;
    mColliders[5].w = 20;
    mColliders[5].h = 6;
    mColliders[6].w = 18;
    mColliders[6].h = 2;
    mColliders[7].w = 16;
    mColliders[7].h = 2;
    mColliders[8].w = 14;
    mColliders[8].h = 1;
    mColliders[9].w = 10;
    mColliders[9].h = 1;
    mColliders[10].w = 6;
    mColliders[10].h = 1;

    // initialize collidesr relative to position
    shiftColliders();
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

void Dot::move(std::vector<SDL_Rect>& otherColliders) {
    // move the dot left or right
    mPosX += mVelX;
    shiftColliders();

    // if the dot collied or went to far to the left or right
    if ((mPosX < 0) || (mPosX + DOT_WIDTH > SCREEN_WIDTH) || checkCollision(mColliders, otherColliders)) {
        // move back;
        mPosX -= mVelX;
        shiftColliders();
    }

    // move the dot up or down
    mPosY += mVelY;
    shiftColliders();

    // if the dot went to far up or down
    if ((mPosY < 0) || (mPosY + DOT_HEIGHT > SCREEN_HEIGHT) || checkCollision(mColliders, otherColliders)) {
        // move back;
        mPosY -= mVelY;
        shiftColliders();
    }
}

void Dot::render() {
    // show the dot
    gDotTexture.render(mPosX, mPosY);
}

void Dot::shiftColliders() {
    // the row offset;
    int r = 0;

    // go through the dot's collision boxes
    for (int set = 0; set < mColliders.size(); ++set) {
        // center the collision box
        mColliders[set].x = mPosX + (DOT_WIDTH - mColliders[set].w) /2;

        // set the collision box at its row offset
        mColliders[set].y = mPosY + r;

        // move the row offset down the height of the collision box
        r += mColliders[set].h;
    }
}

std::vector<SDL_Rect>& Dot::getColliders() {
    return mColliders;
}

bool checkCollision(std::vector<SDL_Rect>& a, std::vector<SDL_Rect>& b) {
    // the sides of the rectangles
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    // go through the A boxes
    for (int Abox = 0; Abox < a.size(); Abox++) {
        // calculate the sides of rect A
        leftA = a[Abox].x;
        rightA = a[Abox].x + a[Abox].w;
        topA = a[Abox].y;
        bottomA = a[Abox].y + a[Abox].h;

        // got through the B boxes
        for (int Bbox = 0; Bbox < b.size(); Bbox++) {
            // calculate the sides of rect A
            leftB = b[Bbox].x;
            rightB = b[Bbox].x + b[Bbox].w;
            topB = b[Bbox].y;
            bottomB = b[Bbox].y + b[Bbox].h;

            // if no sides from A are outside of B
            if (((bottomA <= topB) || (topA >= bottomB) || (rightA <= leftB) || (leftA >= rightB)) == false ) {
                // A collision is detected
                return true;
            }
        }
    }

    // if neither set of collision boxes touched
    return false;
}
