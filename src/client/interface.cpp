#include "interface.h"

Store gStore;

void Interface::Initialize()
{
    font = TTF_OpenFont("C:\\WINDOWS\\Fonts\\ARIAL.TTF", 15);
    TTF_SetFontStyle(font, TTF_STYLE_BOLD);

    SDest = Piskvorky.GetDisplay();

    stage = STAGE_MENU;

    changed = true;
}

void Interface::Draw()
{
    if(!changed)
        return;

    if(!SDest || !font)
        return;

    Surface::DrawRect(SDest,0,0,1024,768,SDL_MapRGB(SDest->format, 0,0,0));

    SetColor(255,255,0);
    Surface::DrawFont(SDest,10,10,font,"Vase ID: %s",gStore.GetName());

    switch(stage)
    {
    case STAGE_MENU:
        SetColor(0,0,0);
        Surface::DrawRect(SDest,60,80,100,25,SDL_MapRGB(SDest->format,255,255,255));
        Surface::DrawFont(SDest,63,83,font,"Nova hra");

        Surface::DrawRect(SDest,60,130,100,25,SDL_MapRGB(SDest->format,255,255,255));
        Surface::DrawFont(SDest,63,133,font,"Konec");
        break;
    case STAGE_CONNECTING:
        SetColor(255,255,255);
        Surface::DrawFont(SDest,63,83,font,"Cekam na protihrace...");
        break;
    case STAGE_GAME:
        break;
    default:
        break;
    }

    changed = false;
}

void Interface::DrawTextRq(int x, int y, const char *text, ...)
{
    Surface::DrawFont(SDest,x,y,font,text);
}

bool inRect(uint32 x, uint32 y, uint32 x1, uint32 x2, uint32 y1, uint32 y2)
{
    return ((x > x1) && (x < x2) && (y > y1) && (y < y2));
}

void Interface::MouseClick(int x, int y, bool left)
{
    if(inRect(x,y,60,160,80,105))
    {
        //Nova hra
        pNetwork->DoConnect();
    }
    if(inRect(x,y,60,160,130,155))
    {
        //Konec
        exit(0);
    }
}

void Store::SetName(std::string newname)
{
    name = newname.c_str();
    Piskvorky.GetInterface()->StoreChanged(); 
}

void Store::SetOponnentName(std::string newname)
{
    oponnentname = newname.c_str();
    Piskvorky.GetInterface()->StoreChanged(); 
}
