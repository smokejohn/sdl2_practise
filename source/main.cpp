#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mouse.h>

#include <cmath>
#include <iostream>
#include <string>

// button constants
const int BUTTON_WIDTH = 300;
const int BUTTON_HEIGHT = 200;
const int TOTAL_BUTTONS = 4;

enum LButtonSprite {
    BUTTON_SPRITE_MOUSE_OUT,
    BUTTON_SPRITE_MOUSE_OVER_MOTION,
    BUTTON_SPRITE_MOUSE_DOWN,
    BUTTON_SPRITE_MOUSE_UP,
    BUTTON_SPRITE_TOTAL
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

// the mouse button
class LButton {
   public:
    // initializes internal variables
    LButton();

    // sets top left position
    void setPosition(int x, int y);

    // handles mouse events
    void handleEvent(SDL_Event* e);

    // show button sprite
    void render();

   private:
    // top left position
    SDL_Point mPosition;

    // currently used global sprite
    LButtonSprite mCurrentSprite;
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

// textures
LTexture gButtonSpriteSheetTexture;
SDL_Rect gSpriteClips[BUTTON_SPRITE_TOTAL];

LButton gButtons[TOTAL_BUTTONS];

// the window renderer
SDL_Renderer* gRenderer = NULL;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

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

LButton::LButton() {
    mPosition.x = 0;
    mPosition.y = 0;

    mCurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
}

void LButton::setPosition(int x, int y) {
    mPosition.x = x;
    mPosition.y = y;
}

void LButton::handleEvent(SDL_Event* e) {
    // if mouse event happened
    if (e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP) {
        // get mouse position
        int x, y;
        SDL_GetMouseState(&x, &y);

        // check if mouse is in button
        bool inside = true;

        // mouse is left of the button
        if (x < mPosition.x) inside = false;
        // mouse is right of the button
        else if (x > mPosition.x + BUTTON_WIDTH)
            inside = false;
        // mouse is above the button
        else if (y < mPosition.y)
            inside = false;
        // mouse is below the button
        else if (y > mPosition.y + BUTTON_HEIGHT)
            inside = false;

        // mouse is outside button
        if (!inside) {
            mCurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
        } else {
            // set mouse over sprite
            switch (e->type) {
                case SDL_MOUSEMOTION:
                    mCurrentSprite = BUTTON_SPRITE_MOUSE_OVER_MOTION;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    mCurrentSprite = BUTTON_SPRITE_MOUSE_DOWN;
                    break;
                case SDL_MOUSEBUTTONUP:
                    mCurrentSprite = BUTTON_SPRITE_MOUSE_UP;
                    break;
            }
        }
    }
}

void LButton::render() {
    // show current button sprite
    gButtonSpriteSheetTexture.render(mPosition.x, mPosition.y, &gSpriteClips[mCurrentSprite]);
}

bool init() {
    // initalization flag
    bool success = true;

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        success = false;
    } else {
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

                // initialize SDL_ttf
                //if (TTF_Init() == -1) {
                //    std::cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << "\n";
                //    success = false;
                //}
            }
        }
    }

    return success;
}

bool loadMedia() {
    // loading success flag
    bool success = true;

    if (!gButtonSpriteSheetTexture.loadFromFile("./resources/button.png")) {
        std::cout << "Unable to load button texture! SDL_Error: " << SDL_GetError() << "\n";
        success = false;
    } else {
        // set sprite clips
        gSpriteClips[0] = {0, 0, 300, 200};
        gSpriteClips[1] = {0, 200, 300, 200};
        gSpriteClips[2] = {0, 400, 300, 200};
        gSpriteClips[3] = {0, 600, 300, 200};

        // init button positions
        gButtons[0].setPosition(0, 0);
        gButtons[1].setPosition(300, 0);
        gButtons[2].setPosition(0, 200);
        gButtons[3].setPosition(300, 200);
    }

    return success;
}

void close() {
    // free loaded images
    gButtonSpriteSheetTexture.free();

    // destroy window
    SDL_DestroyRenderer(gRenderer);
    gRenderer = NULL;
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    // quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
    //TTF_Quit();
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

                    for (int i = 0; i < TOTAL_BUTTONS; i++) {
                        gButtons[i].handleEvent(&e);
                    }
                }

                // clear screen
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                SDL_RenderClear(gRenderer);

                for (int i = 0; i < TOTAL_BUTTONS; i++) {
                    gButtons[i].render();
                }

                // update the screen
                SDL_RenderPresent(gRenderer);
            }
        }
    }

    // free resources and close SDL
    close();

    return 0;
}
