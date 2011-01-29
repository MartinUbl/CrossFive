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
    void DrawFieldElem(unsigned char x, unsigned char y, unsigned char symbol);

    void MouseClick(int x, int y, bool left = true);

    void StoreChanged() { changed = true; };
    void SetStage(Stage nstage) { stage = nstage; };
    Stage GetStage() { return stage; };
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
        for(int i = 0; i < 40; i++)
            for(int j = 0; j < 40; j++)
                field[i][j] = 0;

        myturn = false;
    }

    void SetName(string newname);
    const char* GetName() { return name.c_str(); };
    void SetOponnentName(string newname);
    const char* GetOponnentName() { return oponnentname.c_str(); };

    void SetMyGUID(unsigned int nguid) { myguid = nguid; };
    unsigned int GetMyGUID() { return myguid; };

    unsigned char GetFieldValue(unsigned char x, unsigned char y) { return field[x][y]; };
    void SetFieldValue(unsigned char x, unsigned char y, unsigned char val) { field[x][y] = val; };

    bool IsMyTurn() { return myturn; };
    void SetMyTurn() { myturn = true; };
    void UnsetMyTurn() { myturn = false; };
private:
    std::string name;
    std::string oponnentname;

    unsigned int myguid;
    unsigned char field[40][40];

    bool myturn;
};

extern Store gStore;

#endif
