#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class LWindow {
   public:
    // initializes internals
    LWindow();

    // creates window
    bool init();

    // handles window events
    void handleEvent(SDL_Event& e);

    // focuses on window
    void focus();

    // shows windows contents
    void render();

    // deallocates internals
    void free();

    // window dimensions
    int getWidth();
    int getHeight();

    // window focii
    bool hasMouseFocus();
    bool hasKeyboardFocus();
    bool isMinimized();
    bool isShown();

   private:
    // window data
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    int mWindowID;
    int mWindowDisplayID;

    // window dimensions
    int mWidth;
    int mHeight;

    // window focus
    bool mMouseFocus;
    bool mKeyboardFocus;
    bool mFullScreen;
    bool mMinimized;
    bool mShown;
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

// our custom window
LWindow gWindow;

// display data
int gTotalDisplays = 0;
SDL_Rect* gDisplayBounds = NULL;

// the window renderer
SDL_Renderer* gRenderer = NULL;

// font
TTF_Font* gFont;

// screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

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

        // get number of displays
        gTotalDisplays = SDL_GetNumVideoDisplays();
        if (gTotalDisplays < 2 ) {
            std::cout << "Warning: Only one display connected!\n";
        }

        // get bounds of each display
        gDisplayBounds = new SDL_Rect[gTotalDisplays];
        for ( int i = 0; i < gTotalDisplays; ++i) {
            SDL_GetDisplayBounds(i, &gDisplayBounds[i]);
        }

        // create window
        if (!gWindow.init()) {
            std::cout << "Window could not be created!\n";
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
    return success;
}

bool loadMedia() {
    // loading success flag
    bool success = true;

    return success;
}

void close() {
    // destroy windows
    SDL_DestroyRenderer(gRenderer);

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

            // while application is running
            while (!quit) {
                // handle events on queue
                while (SDL_PollEvent(&e) != 0) {
                    // user requests quit
                    if (e.type == SDL_QUIT) {
                        quit = true;
                    }

                    // handle window events
                    gWindow.handleEvent(e);
                }

                // update window
                gWindow.render();
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

LWindow::LWindow() {
    // initialize non-existant window
    mWindow = NULL;
    mMouseFocus = false;
    mKeyboardFocus = false;
    mFullScreen = false;
    mMinimized = false;
    mWidth = 0;
    mHeight = 0;
}

bool LWindow::init() {
    // create window
    mWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                               SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (mWindow != NULL) {
        mMouseFocus = true;
        mKeyboardFocus = true;
        mWidth = SCREEN_WIDTH;
        mHeight = SCREEN_HEIGHT;

        // create renderer for window
        mRenderer = SDL_CreateRenderer(mWindow, 01, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (mRenderer == NULL) {
            std::cout << "Renderer could not be created! Sdl Error: " << SDL_GetError() << "\n";
            mWindow = NULL;
        } else {
            // initialize renderer color
            SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);

            // grab window identifiers
            mWindowID = SDL_GetWindowID(mWindow);
            mWindowDisplayID = SDL_GetWindowDisplayIndex(mWindow);

            // flag as opened
            mShown = true;
        }
    } else {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
    }

    return mWindow != NULL && mRenderer != NULL;
}

void LWindow::handleEvent(SDL_Event& e) {
    // caption update flag
    bool updateCaption = false;

    // window event occured
    if (e.type == SDL_WINDOWEVENT && e.window.windowID == mWindowID) {
        switch (e.window.event) {
            // window moved
            case SDL_WINDOWEVENT_MOVED:
                mWindowDisplayID = SDL_GetWindowDisplayIndex(mWindow);
                updateCaption = true;
                break;

            // window appeared
            case SDL_WINDOWEVENT_SHOWN:
                mShown = true;
                break;

            // window disappeared
            case SDL_WINDOWEVENT_HIDDEN:
                mShown = false;
                break;

            // get new dimensions and repaint on window size change
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                mWidth = e.window.data1;
                mHeight = e.window.data2;
                SDL_RenderPresent(gRenderer);
                break;

            // repaint on exposure
            case SDL_WINDOWEVENT_EXPOSED:
                SDL_RenderPresent(gRenderer);
                break;

            // mouse entered window
            case SDL_WINDOWEVENT_ENTER:
                mMouseFocus = true;
                updateCaption = true;
                break;

            // mouse left window
            case SDL_WINDOWEVENT_LEAVE:
                mMouseFocus = false;
                updateCaption = true;
                break;

            // window has keyboard focus
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                mKeyboardFocus = true;
                updateCaption = true;
                break;

            // window lost keyboard focus
            case SDL_WINDOWEVENT_FOCUS_LOST:
                mKeyboardFocus = false;
                updateCaption = true;
                break;

            // window minimized
            case SDL_WINDOWEVENT_MINIMIZED:
                mMinimized = true;
                break;

            // window maximized
            case SDL_WINDOWEVENT_MAXIMIZED:
                mMinimized = false;
                break;

            // window restored
            case SDL_WINDOWEVENT_RESTORED:
                mMinimized = false;
                break;

            // hide window on close
            case SDL_WINDOWEVENT_CLOSE:
                SDL_HideWindow(mWindow);
                break;
        }
    } else if (e.type == SDL_KEYDOWN) {
        // display change flag
        bool switchDisplay = false;

        // cycle through displays on up/down
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                ++mWindowDisplayID;
                switchDisplay = true;
                break;

            case SDLK_DOWN:
                --mWindowDisplayID;
                switchDisplay = true;
                break;
        }
        // display needs to be updated
        if (switchDisplay) {
            // bound display index
            if (mWindowDisplayID < 0) {
                mWindowDisplayID = gTotalDisplays - 1;
            } else if (mWindowDisplayID >= gTotalDisplays) {
                mWindowDisplayID = 0;
            }

            // move window to center of next display
            SDL_Rect bounds = gDisplayBounds[mWindowDisplayID];
            SDL_SetWindowPosition(mWindow, bounds.x + (bounds.w - mWidth) / 2, bounds.y + (bounds.h - mHeight) / 2);
            updateCaption = true;
        }
    }
    // update window caption with new data
    if (updateCaption) {
        std::stringstream caption;
        caption << "SDL Tutorial - ID: " << mWindowID << " Display: " << mWindowDisplayID
                << " MouseFocus:" << ((mMouseFocus) ? "On" : "Off")
                << " KeyboardFocus:" << ((mKeyboardFocus) ? "On" : "Off");
        SDL_SetWindowTitle(mWindow, caption.str().c_str());
    }
}

void LWindow::free() {
    if (mWindow != NULL) {
        SDL_DestroyWindow(mWindow);
    }
    mMouseFocus = false;
    mKeyboardFocus = false;
    mWidth = 0;
    mHeight = 0;
}

void LWindow::focus() {
    // restore window if needed
    if (!mShown) {
        SDL_ShowWindow(mWindow);
    }

    // move window forward
    SDL_RaiseWindow(mWindow);
}

void LWindow::render() {
    if (!mMinimized) {
        // clear screen
        SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
        SDL_RenderClear(mRenderer);

        // update screen
        SDL_RenderPresent(mRenderer);
    }
}

int LWindow::getWidth() { return mWidth; }

int LWindow::getHeight() { return mHeight; }

bool LWindow::hasMouseFocus() { return mMouseFocus; }

bool LWindow::hasKeyboardFocus() { return mKeyboardFocus; }

bool LWindow::isMinimized() { return mMinimized; }

bool LWindow::isShown() { return mShown; }
