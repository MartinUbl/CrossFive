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

void Interface::DrawFieldElem(unsigned char x, unsigned char y, unsigned char symbol)
{
    uint32 realx, realy;

    //10+2 kvuli ohraniceni, +2 kvuli tomu aby nesplyval
    realx = 10+2+2+x*(15+2);
    realy = 45+2+2+y*(15+2);

    switch(symbol)
    {
    case 1: //krizek
        Surface::DrawRect(SDest,realx,realy,11,11,SDL_MapRGB(SDest->format,255,0,0));
        break;
    case 2: //kolecko
        Surface::DrawRect(SDest,realx,realy,11,11,SDL_MapRGB(SDest->format,0,255,0));
        break;
    default: //prazdno (0 a jine)
        Surface::DrawRect(SDest,realx,realy,11,11,SDL_MapRGB(SDest->format,0,0,0));
        break;
    }
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
        Surface::DrawFont(SDest,10,25,font,"ID protihrace: %s",gStore.GetOponnentName());
        Surface::DrawRect(SDest,10,45,2+40*(2+15),2,SDL_MapRGB(SDest->format,255,255,255));
        Surface::DrawRect(SDest,10,45,2,2+40*(2+15),SDL_MapRGB(SDest->format,255,255,255));
        for(int i = 0; i < 40; i++)
        {
            Surface::DrawRect(SDest,10,45+2+15+i*(2+15),2+40*(2+15),2,SDL_MapRGB(SDest->format,255,255,255));
            Surface::DrawRect(SDest,10+2+15+i*(2+15),45,2,2+40*(2+15),SDL_MapRGB(SDest->format,255,255,255));
            for(int j = 0; j < 40; j++)
                DrawFieldElem(i,j,gStore.GetFieldValue(i,j));
        }
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
    if(inRect(x,y,60,160,80,105) && stage == STAGE_MENU)
    {
        //Nova hra
        pNetwork->DoConnect();
    }
    if(inRect(x,y,60,160,130,155) && stage == STAGE_MENU)
    {
        //Konec
        exit(0);
    }
    if(inRect(x,y,10+2,10+2+2+40*(2+15),45+2,45+2+40*(2+15)) && stage == STAGE_GAME)
    {
        //Tah
        if(!gStore.IsMyTurn())
            return;

        unsigned char field_x, field_y;

        field_x = (unsigned char)(int(x-10) / int(2+15));
        field_y = (unsigned char)(int(y-45) / int(2+15));

        GamePacket data(CMSG_TURN);
        data << field_x;
        data << field_y;
        SendToServer(&data);
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
