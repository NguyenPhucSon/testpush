#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <stdlib.h>
#include <ctime>
#include <SDL_mixer.h>

using namespace std;

vector<int> xCor {0, 1, 1, 1, 0, -1, -1, -1};
vector<int> yCor {-1, -1, 0, 1, 1, 1, 0, -1};

const int SCREEN_WIDTH = 488;
const int SCREEN_HEIGHT = 580;

int actualMineRemain = 40;
int mineRemain = 40;

int nof = 0;

bool endGame = false;

class LTexture
{
    public:
        LTexture();
        ~LTexture();

        bool loadFromFile (string path);

        void Free();

        void render (int x, int y, SDL_Rect* Clips = NULL);

        int getWidth();
        int getHeight();

    private:
        SDL_Texture* mTexture;
        int mWidth;
        int mHeight;

};

struct Button
{
    int value;
    bool revealable;
    bool markable;
};

void reveal(int x, int y);

void mark(int x, int y);

void gameOver();

bool init();

bool loadImage();

void SDL_Collapse();

void Setup();

void createMap ();

int checkAround (int x, int y);

void replay();

SDL_Texture* loadTexture(string path);

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Texture* background = NULL;

Mix_Chunk *gScratch = NULL;
Mix_Chunk *gClick = NULL;
Mix_Chunk *gBeat = NULL;

LTexture gSpriteSheetTexture;
SDL_Rect gSpriteClips[12];

LTexture displayCounting;
SDL_Rect displayMineRemain[10];

LTexture faceP;
LTexture faceL;
SDL_Rect playingFace;
SDL_Rect losingFace;

int mineField [14][14];
Button mineMap [14][14];

LTexture::LTexture()
{
    mWidth = 0;
    mHeight = 0;
    mTexture = NULL;
}

LTexture::~LTexture()
{
    Free();
}

bool LTexture::loadFromFile(string path)
{
    Free();

    SDL_Texture* newTexture = NULL;

    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == NULL) {
        cout<<"Unable to load surface. ERROR: \n"<<IMG_GetError()<<endl;
    }
    else {
        newTexture = SDL_CreateTextureFromSurface (gRenderer, loadedSurface);
    }
    if (newTexture == NULL) {
        cout<<"Unable to create texture frome surface \n"<<SDL_GetError()<<endl;
    }
    else {
        mWidth = loadedSurface->w;
        mHeight = loadedSurface->h;
    }
    mTexture = newTexture;
    return mTexture!=NULL;
}

void LTexture::Free ()
{
    if (mTexture != NULL) {
        SDL_DestroyTexture(mTexture);
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
    }
}

void LTexture::render (int x, int y, SDL_Rect* Clips)
{
    SDL_Rect renderQuad = {x, y, mWidth, mHeight};
    if (Clips != NULL) {
        renderQuad.w = Clips->w;
        renderQuad.h = Clips ->h;
    }
    SDL_RenderCopy (gRenderer, mTexture, Clips, &renderQuad);
}

int LTexture ::getWidth()
{
    return mWidth;
}

int LTexture::getHeight()
{
    return mHeight;
}

bool init()
{
    if (SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0) {
        cout<<"Failed to init video. \n"<<SDL_GetError()<<endl;
        return false;
    }
    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")<0) {
        cout<<"Warning: Linear texture filtering not enabled!"<<endl;
    }
    gWindow = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == NULL){
        cout<<"Failed to create window \n"<<SDL_GetError();
        return false;
    }
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (gRenderer == NULL) {
        cout<<"Failed to create renderer \n"<<SDL_GetError()<<endl;
        return false;
    }
    SDL_SetRenderDrawColor (gRenderer, 0x00, 0x00, 0xFF, 0xFF);
    int flag = IMG_INIT_PNG;
    if (!(IMG_Init(flag) & flag)) {
        cout<<"Failed to init img \n"<<IMG_GetError()<<endl;
        return false;
    }

    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        return false;
    }
    return true;
}

bool loadImage ()
{
    if (!gSpriteSheetTexture.loadFromFile("res/image/Tiles.png")) {
        cout<<"Failed to load spirite sheet from texture \n";
        return false;
    } else {
        for (int i = 0; i<12; i++){
            gSpriteClips[i].x = 32*i;
            gSpriteClips[i].y = 0;
            gSpriteClips[i].w = 32;
            gSpriteClips[i].h = 32;
        }
    }
    if (!displayCounting.loadFromFile("res/image/Count.png")) {
        cout<<"Failed to counting \n";
        return false;
    } else {
        for (int i = 0; i<10; i++) {
            displayMineRemain[i].x = 28*i;
            displayMineRemain[i].y = 0;
            displayMineRemain[i].w = 28;
            displayMineRemain[i].h = 46;
        }
    }

    if (!faceP.loadFromFile("res/image/playingface.png")) {
        cout<<"Failed to counting \n";
        return false;
    } else {
        playingFace.x = 0;
        playingFace.y = 0;
        playingFace.w = 42;
        playingFace.h = 42;
    }

    if (!faceL.loadFromFile("res/image/loseface.png")) {
        cout<<"Failed to counting \n";
        return false;
    } else {
        losingFace.x = 0;
        losingFace.y = 0;
        losingFace.w = 42;
        losingFace.h = 42;
    }

    background = loadTexture("res/image/medium.png");
    if (background == NULL) {
        cout<<"Failed to load background \n"<<IMG_GetError()<<endl;
        return false;
    }

    gBeat = Mix_LoadWAV( "res/sound/beat.wav" );
	if( gBeat == NULL )
	{
		printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
		return false;
	}

	gClick = Mix_LoadWAV( "res/sound/click.wav" );
	if( gClick == NULL )
	{
		printf( "Failed to load click music! SDL_mixer Error: %s\n", Mix_GetError() );
		return false;
	}

	gScratch = Mix_LoadWAV( "res/sound/scratch2.wav" );
	if( gScratch == NULL )
	{
		printf( "Failed to load scratch music! SDL_mixer Error: %s\n", Mix_GetError() );
		return false;
	}

    return true;
}

void SDL_Collapse()
{
    gSpriteSheetTexture.Free();
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gRenderer = NULL;
    gWindow = NULL;

    Mix_FreeChunk(gScratch);
    Mix_FreeChunk(gBeat);
    Mix_FreeChunk(gClick);
    gClick = NULL;
    gBeat = NULL;
    gScratch = NULL;

    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

void Setup()
{
    endGame = false;
    actualMineRemain = 40;
    mineRemain = 40;
    nof = 0;
    for (int i=0; i<14; i++){
        for (int j = 0; j<14; j++){
            mineField[i][j] = 0;
        }
    }
    int x = 0;
    int y = 0;
    srand(time(NULL));
    for (int i=0; i<40; i++){
        do{
            x = rand() % 13 + 0;
            y = rand() % 13 + 0;
        } while (mineField [x][y] != 0);
        mineField[x][y] = 1;
    }
}

void createMap()
{
    for (int i=0; i<14; i++) {
        for (int j=0; j<14; j++){
            if (mineField[i][j] == 0){
                mineMap[i][j].value = checkAround(j, i);
            } else {
                mineMap[i][j].value = -1;
            }
            mineMap[i][j].markable = true;
            mineMap[i][j].revealable = true;
        }
    }
}

int checkAround(int x, int y)
{
    int s = 0;
    for (int k=0; k<8; k++){
        int j = x + xCor[k];
        int i = y + yCor[k];
        if ( i<0 || j<0 || i>=14 || j>=14 ) continue;
        else{
            if (mineField[i][j] == 1) s++;
        }
    }
    return s;
}

void reveal(int x, int y)
{
    if (mineMap[y][x].revealable){
        if (mineMap[y][x].markable){
            int m = mineMap[y][x].value;
            if (m==-1){
                Mix_PlayChannel(-1, gScratch, 0);
                gameOver();
            }
            else if (m>0){
                gSpriteSheetTexture.render(19+x*32, 112+y*32, &gSpriteClips[m]);
                SDL_RenderPresent(gRenderer);
                mineMap[y][x].revealable = false;
            }
            else {
                gSpriteSheetTexture.render(19+x*32, 112+y*32, &gSpriteClips[0]);
                mineMap[y][x].revealable = false;
                SDL_RenderPresent(gRenderer);
                for (int k=0; k<8; k++){
                    int i = y + yCor[k];
                    int j = x + xCor[k];
                    if (i<0 || j<0 || i>=14 || j>=14){
                        continue;
                    }
                    else{
                        reveal(j, i);
                    }
                }
            }
        }
    }
}

void mark(int x, int y)
{
        if (mineMap[y][x].revealable){
            if (mineMap[y][x].markable){
                gSpriteSheetTexture.render(19+x*32, 112+y*32, &gSpriteClips[11]);
                mineMap[y][x].markable = false;
                mineRemain--;
                if (mineMap[y][x].value == -1) {
                    actualMineRemain--;
                }
            }
            else {
                gSpriteSheetTexture.render(19+x*32, 112+y*32, &gSpriteClips[10]);
                mineMap[y][x].markable = true;
                mineRemain++;
                if (mineMap[y][x].value == -1) {
                    actualMineRemain++;
                }
            }

            int strt = mineRemain / 10;
            int en = mineRemain % 10;

            if (mineRemain > 0) {
                displayCounting.render(SCREEN_WIDTH - 92, 33, &displayMineRemain[strt]);
                displayCounting.render(SCREEN_WIDTH - 64, 33, &displayMineRemain[en]);
            } else {
                displayCounting.render(SCREEN_WIDTH - 92, 33, &displayMineRemain[0]);
                displayCounting.render(SCREEN_WIDTH - 64, 33, &displayMineRemain[0]);

            }
            SDL_RenderPresent(gRenderer);
        }
}

void gameOver()
{
    mineRemain = -1;
    for (int i = 0; i<14; i++) {
        for (int j = 0; j<14; j++) {
            mineMap[i][j].markable = false;
            mineMap[i][j].revealable = false;
            if (mineMap[i][j].value == -1) {
                gSpriteSheetTexture.render(19+j*32, 112+i*32, &gSpriteClips[9]);
                SDL_RenderPresent(gRenderer);
            }
        }
    }
    endGame = true;
}

void replay()
{
    Setup();
    createMap();
    SDL_SetRenderDrawColor(gRenderer, 165, 165, 150, 0xFF);
    SDL_RenderClear(gRenderer);
    SDL_RenderCopy(gRenderer, background, NULL, NULL);
    faceP.render((SCREEN_WIDTH - 42) / 2, 35, &playingFace);

    for (int i=0; i<14; i++) {
        for (int j = 0; j<14; j++) {
            gSpriteSheetTexture.render(19+j*32, 112+i*32, &gSpriteClips[10]);
        }
    }
    displayCounting.render(SCREEN_WIDTH - 92, 33, &displayMineRemain[mineRemain / 10]);
    displayCounting.render(SCREEN_WIDTH - 64, 33, &displayMineRemain[mineRemain % 10]);

    SDL_RenderPresent(gRenderer);
}

SDL_Texture* loadTexture(string path)
{
    SDL_Texture* newTexture = NULL;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == NULL) {
        cout<<"Unable to load surface"<<endl;
    } else {
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == NULL) {
            cout<<"Unable to create texture. \n"<<SDL_GetError()<<endl;
        }
    }
    SDL_FreeSurface(loadedSurface);
    return newTexture;
}

int main (int argc, char* argv[])
{
    int x = -1;
    int y = -1;
    int x2 = 0, y2 = 0;
    Setup();
    createMap();
    if (!init()) {
        cout<<"Failed to init \n";
    } else {
        if (!loadImage()) {
            cout<<"Failed to load image \n";
        } else {
            SDL_RenderClear(gRenderer);
            SDL_RenderCopy(gRenderer, background, NULL, NULL);

            faceP.render((SCREEN_WIDTH - 42) / 2, 35, &playingFace);
            for (int i=0; i<14; i++) {
                for (int j = 0; j<14; j++) {
                    gSpriteSheetTexture.render(19+j*32, 112+i*32, &gSpriteClips[10]);
                }
            }

            displayCounting.render(SCREEN_WIDTH - 92, 33, &displayMineRemain[mineRemain / 10]);
            displayCounting.render(SCREEN_WIDTH - 64, 33, &displayMineRemain[mineRemain % 10]);

            SDL_RenderPresent(gRenderer);
            bool isRunning = true;
            SDL_Event mainEvent;

            while (isRunning) {
                while (SDL_PollEvent( &mainEvent )!=0) {

                    if (mainEvent.type == SDL_QUIT) {
                        isRunning = false;
                    }

                    if (mainEvent.type == SDL_KEYDOWN) {
                        if (mainEvent.key.keysym.sym == SDLK_ESCAPE) isRunning = false;
                    }

                    if (actualMineRemain == 0 && nof == 0) {
                        Mix_PlayChannel(-1, gBeat, 0);
                        nof = -1;
                    }

                    if (mineRemain < 0) {
                        faceL.render((SCREEN_WIDTH - 42) / 2, 35, &losingFace);
                        SDL_RenderPresent(gRenderer);
                    } else {
                        faceP.render((SCREEN_WIDTH - 42) / 2, 35, &playingFace);
                        SDL_RenderPresent(gRenderer);
                    }

                    if (mainEvent.type == SDL_MOUSEMOTION) {
                        x2 = mainEvent.motion.x;
                        y2 = mainEvent.motion.y;
                        x = (mainEvent.motion.x - 19)/32;
                        y = (mainEvent.motion.y - 112)/32;
                    }

                    if (mainEvent.type == SDL_MOUSEBUTTONDOWN && (x>=0 && x<14 && y>=0 && y<14)) {
                        if (mainEvent.button.button == SDL_BUTTON_RIGHT) {
                            mark(x, y);
                        }
                        if (mainEvent.button.button == SDL_BUTTON_LEFT) {
                            reveal(x, y);
                        }

                        Mix_PlayChannel(-1, gClick, 0);
                    }

                    if (mainEvent.type == SDL_MOUSEBUTTONDOWN && !(x>=0 && x<14 && y>=0 && y<14)) {
                        if (mainEvent.button.button == SDL_BUTTON_LEFT) {
                            if (x2>=(SCREEN_WIDTH - 42) / 2 && x2<=((SCREEN_WIDTH - 42) / 2) + 42 && y2>=35 && y2<=77) {
                                replay();
                            }
                        }
                    }
                    if (endGame) {
                        faceL.render((SCREEN_WIDTH - 42) / 2, 35, &losingFace);
                        SDL_RenderPresent(gRenderer);
                    }
                }
            }
        }
    }
    SDL_Collapse();
    return 0;
}
