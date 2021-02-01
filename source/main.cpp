#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// particle count
const int TOTAL_PARTICLES = 20;

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

   private:
    // the actual hardware texture
    SDL_Texture* mTexture;

    // image dimensions
    int mWidth;
    int mHeight;
};


class Particle {
   public:
    // initialize position and animation
    Particle(int x, int y);

    // shows the particle
    void render();

    // check if particle is dead
    bool isDead();
   private:
    // offsets
    int mPosX, mPosY;

    // current frame of animation
    int mFrame;

    // type of marticle
    LTexture* mTexture;
};

// the dot that will move around the screen
class Dot {
   public:
    // the dimensions of the dot
    static const int DOT_WIDTH = 20;
    static const int DOT_HEIGHT = 20;

    // maximum axis velocity of the dot
    static const int DOT_VEL = 10;

    // initializes the variables and allocates particles
    Dot();

    // deallocates particles
    ~Dot();

    // takes key presses and adjusts the dot's velocity
    void handleEvent(SDL_Event& e);

    // modes the dot
    void move();

    // shows the dot on the screen
    void render();

   private:
    // the particles
    Particle* particles[TOTAL_PARTICLES];

    // shows the particles
    void renderParticles();

    // the X and Y offsets of the dot
    int mPosX, mPosY;

    // the velocity of the dot
    int mVelX, mVelY;
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
LTexture gDotTexture;
LTexture gRedTexture;
LTexture gGreenTexture;
LTexture gBlueTexture;
LTexture gShimmerTexture;

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

    // load dot texture
    if (!gDotTexture.loadFromFile("./resources/images/dot.bmp")) {
        std::cout << "Failed to load dot texture!\n";
        success = false;
    }

    if (!gRedTexture.loadFromFile("./resources/images/red.bmp")) {
        std::cout << "Failed to load red texture!\n";
        success = false;
    }

    if (!gGreenTexture.loadFromFile("./resources/images/green.bmp")) {
        std::cout << "Failed to load green texture!\n";
        success = false;
    }

    if (!gBlueTexture.loadFromFile("./resources/images/blue.bmp")) {
        std::cout << "Failed to load blue texture!\n";
        success = false;
    }

    if (!gShimmerTexture.loadFromFile("./resources/images/shimmer.bmp")) {
        std::cout << "Failed to load shimmer texture!\n";
        success = false;
    }

    // set texture transparency
    gRedTexture.setAlpha(192);
    gGreenTexture.setAlpha(192);
    gBlueTexture.setAlpha(192);
    gShimmerTexture.setAlpha(192);

    return success;
}

void close() {
    // destroy windows
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gRenderer = NULL;
    gWindow = NULL;

    // destroy data
    gDotTexture.free();
    gRedTexture.free();
    gGreenTexture.free();
    gBlueTexture.free();
    gShimmerTexture.free();

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

            // the dot
            Dot dot;

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

                    // handle input for the dot
                    dot.handleEvent(e);
                }

                // move the dot
                dot.move();

                // clear screen
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                SDL_RenderClear(gRenderer);

                // render objects
                dot.render();

                // update the screen
                SDL_RenderPresent(gRenderer);
            }
        }
    }

    // free resources and close SDL
    close();

    return 0;
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

Particle::Particle(int x, int y) {
    // set offsets
    mPosX = x - 5 + (rand() % 25);
    mPosY = y - 5 + (rand() % 25);

    // initialize animation
    mFrame = rand() % 5;

    // set type
    switch (rand() % 3) {
        case 0:
            mTexture = &gRedTexture;
            break;
        case 1:
            mTexture = &gGreenTexture;
            break;
        case 2:
            mTexture = &gBlueTexture;
            break;
    }
}

void Particle::render() {
    // show image
    mTexture->render(mPosX, mPosY);

    // show shimmer
    if (mFrame % 2 == 0) {
        gShimmerTexture.render(mPosX, mPosY);
    }

    // animate
    mFrame++;
}

bool Particle::isDead() { return mFrame > 10; }

Dot::Dot() {
    // initialize the offsets
    mPosX = 0;
    mPosY = 0;

    // initialize the velocity
    mVelX = 0;
    mVelY = 0;

    // initialize particles
    for (int i = 0; i < TOTAL_PARTICLES; ++i) {
        particles[i] = new Particle(mPosX, mPosY);
    }
}

Dot::~Dot() {
    // delete particles
    for (int i = 0; i < TOTAL_PARTICLES; ++i) {
        delete particles[i];
    }
}

void Dot::render() {
    // show the dot
    gDotTexture.render(mPosX, mPosY);

    // show the particles
    renderParticles();
}

void Dot::move() {
    // move the dot left or right
    mPosX += mVelX;

    // if the dot went too far to the left or right
    if ((mPosX < 0) || (mPosX + DOT_WIDTH > SCREEN_WIDTH)) {
        // move back
        mPosX -= mVelX;
    }

    // move the dot up or down
    mPosY += mVelY;

    // if the dot went too far up or down
    if ((mPosY < 0) || (mPosY + DOT_HEIGHT > SCREEN_HEIGHT)) {
        // move back
        mPosY -= mVelY;
    }
}

void Dot::handleEvent(SDL_Event& e) {
    // If a key was pressed
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        // Adjust the velocity
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
    // If a key was released
    else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
        // Adjust the velocity
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

void Dot::renderParticles() {
    // go through the particles
    for (int i = 0; i < TOTAL_PARTICLES; ++i) {
        // delete and replace dead particles
        if (particles[i]->isDead()) {
            delete particles[i];
            particles[i] = new Particle(mPosX, mPosY);
        }
    }

    // show particles
    for (int i = 0; i < TOTAL_PARTICLES; ++i) {
        particles[i]->render();
    }
}
