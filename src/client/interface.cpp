#include "interface.h"

Store gStore;

//Initialize interface
void Interface::Initialize()
{
    //Opens and uses fonts
    font = TTF_OpenFont("C:\\WINDOWS\\Fonts\\ARIAL.TTF", 15);
    TTF_SetFontStyle(font, TTF_STYLE_BOLD);

    //Get default surface
    SDest = Piskvorky.GetDisplay();

    //Set stage to menu
    stage = STAGE_MENU;

    changed = true;
}

//Draw field (blank, cross or circle)
void Interface::DrawFieldElem(unsigned char x, unsigned char y, unsigned char symbol)
{
    uint32 realx, realy;

    //10+2 for border, +2 to avoid mergence
    realx = 10+2+2+x*(15+2);
    realy = 45+2+2+y*(15+2);

    switch(symbol)
    {
    case 1: //cross
        Surface::DrawRect(SDest,realx,realy,11,11,SDL_MapRGB(SDest->format,255,0,0));
        break;
    case 2: //circle
        Surface::DrawRect(SDest,realx,realy,11,11,SDL_MapRGB(SDest->format,0,255,0));
        break;
    default: //blank (0 and other)
        Surface::DrawRect(SDest,realx,realy,11,11,SDL_MapRGB(SDest->format,0,0,0));
        break;
    }
}

//Drawing function
void Interface::Draw()
{
    //if nothing changed, just return
    if(!changed)
        return;

    //If wrong destination surface or font not initialized, return
    if(!SDest || !font)
        return;

    //clear screen (black)
    Surface::DrawRect(SDest,0,0,1024,768,SDL_MapRGB(SDest->format, 0,0,0));

    //Set default color to yellow
    SetColor(255,255,0);

    //Write our ID
    Surface::DrawFont(SDest,10,10,font,"Vase ID: %s",gStore.GetName());

    //And draw stage-specific stuff
    switch(stage)
    {
    case STAGE_MENU: //For menu stage
        SetColor(0,0,0);
        Surface::DrawRect(SDest,60,80,100,25,SDL_MapRGB(SDest->format,255,255,255));
        Surface::DrawFont(SDest,63,83,font,"Nova hra");

        Surface::DrawRect(SDest,60,130,100,25,SDL_MapRGB(SDest->format,255,255,255));
        Surface::DrawFont(SDest,63,133,font,"Konec");
        break;
    case STAGE_CONNECTING: //For waiting stage (Waiting for oponnent...)
        SetColor(255,255,255);
        Surface::DrawFont(SDest,63,83,font,"Cekam na protihrace...");
        break;
    case STAGE_GAME: //For main game
        Surface::DrawFont(SDest,10,25,font,"ID protihrace: %s",gStore.GetOponnentName());
        //Draw first two lines for frame
        Surface::DrawRect(SDest,10,45,2+40*(2+15),2,SDL_MapRGB(SDest->format,255,255,255));
        Surface::DrawRect(SDest,10,45,2,2+40*(2+15),SDL_MapRGB(SDest->format,255,255,255));
        //And draw whole field
        for(int i = 0; i < 40; i++)
        {
            //Lines...
            Surface::DrawRect(SDest,10,45+2+15+i*(2+15),2+40*(2+15),2,SDL_MapRGB(SDest->format,255,255,255));
            Surface::DrawRect(SDest,10+2+15+i*(2+15),45,2,2+40*(2+15),SDL_MapRGB(SDest->format,255,255,255));
            //Draw field values (blank, cross or circle)
            for(int j = 0; j < 40; j++)
                DrawFieldElem(i,j,gStore.GetFieldValue(i,j));
        }
        break;
    default:
        break;
    }

    //And after drawing, set changed to false, to avoid re-drawing the same
    changed = false;
}

//Unused, about to remove
void Interface::DrawTextRq(int x, int y, const char *text, ...)
{
    Surface::DrawFont(SDest,x,y,font,text);
}

//Is point in rectange?
bool inRect(uint32 x, uint32 y, uint32 x1, uint32 x2, uint32 y1, uint32 y2)
{
    return ((x > x1) && (x < x2) && (y > y1) && (y < y2));
}

//Interface handling of mouse click
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

//Set our name
void Store::SetName(std::string newname)
{
    name = newname.c_str();
    Piskvorky.GetInterface()->StoreChanged(); 
}

//Set oponnent name
void Store::SetOponnentName(std::string newname)
{
    oponnentname = newname.c_str();
    Piskvorky.GetInterface()->StoreChanged(); 
}
