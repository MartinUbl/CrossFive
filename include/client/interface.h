#ifndef __INTERFACE_H_
#define __INTERFACE_H_

#include <global.h>

typedef enum
{
    STAGE_MENU,
    STAGE_CONNECTING,
    STAGE_GAME,
} Stage;

class Network;

class Interface
{
public:
    void Initialize();
    void Draw();

    void SetNetwork(Network* nwork) { pNetwork = nwork; };

    TTF_Font* GetFont() { return font; };
    SDL_Surface* GetSurface() { return SDest; };

    void DrawTextRq(int x, int y, const char* text, ...);

    void StoreChanged() { changed = true; };
private:
    Network* pNetwork;
    SDL_Surface* SDest;
    TTF_Font* font;
    bool changed;

    Stage stage;
};

class Store
{
public:
    Store()
    {
        name = "";
    }

    void SetName(string newname);
    const char* GetName() { return name.c_str(); };
private:
    std::string name;
};

extern Store gStore;

#endif
