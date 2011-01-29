#include <global.h>
#include <cstdarg>

Application Piskvorky;
SDL_Color defcolor = {255,255,0,0};

//Main function (SDL_main is defined as main entry point)
int SDL_main(int argc, char *argv[]) 
{
    srand((unsigned int)time(NULL));

    Interface* pInterface = new Interface;
    Network* pNetwork = new Network;

    pInterface->SetNetwork(pNetwork);

    Piskvorky.SetInterface(pInterface);

    return Piskvorky.OnExecute();
}

//set global color
void SetColor(unsigned int r, unsigned int g, unsigned int b)
{
    defcolor.r = r;
    defcolor.g = g;
    defcolor.b = b;
}

//On application initialize - returns false if something fails
bool Application::OnInit()
{
    //Init main SDL lib
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
        return false;

    //Startup SDL_TTF library
    if(TTF_Init() < 0)
        return false;

    //Set surface
    if((DrawSurface = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE | SDL_DOUBLEBUF)) == NULL)
        return false;

    //Initialize interface as well
    pInterface->Initialize();

    return true;
}

//Main loop
int Application::OnExecute()
{
    if(!OnInit())
        return -1;

    Running = true;

    SDL_Event Event;

    while(Running)
    {
        while(SDL_PollEvent(&Event))
        {
            //process events
            OnEvent(&Event);
        }

        //let interface draw
        pInterface->Draw();
        SDL_Flip(DrawSurface);
    }

    //kill running thread
    if(net_thread)
        SDL_KillThread(net_thread);

    SDL_FreeSurface(DrawSurface);
    SDL_Quit();

    return 0;
}

//Event handling
void Application::OnEvent(SDL_Event* Event) 
{
    switch(Event->type)
    {
        case SDL_QUIT: //cross pressed
            Running = false;
            break;
        case SDL_MOUSEBUTTONDOWN: //mouse button pressed
            if(Event->button.button == SDL_BUTTON_LEFT)
                pInterface->MouseClick(Event->button.x, Event->button.y, true);
            else
                pInterface->MouseClick(Event->button.x, Event->button.y, false);
            break;
    }
}

//constructor, unused
Surface::Surface()
{
}

//Load image as surface
SDL_Surface* Surface::Load(const char* File)
{
    SDL_Surface* Surf_Temp = NULL;
    SDL_Surface* Surf_Return = NULL;

    if((Surf_Temp = IMG_Load(File)) == NULL)
        return NULL;

    Surf_Return = SDL_DisplayFormat(Surf_Temp);
    SDL_FreeSurface(Surf_Temp);

    return Surf_Return;
}

//Draw surface to surface
bool Surface::Draw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y, int W, int H)
{
    if(Surf_Dest == NULL || Surf_Src == NULL)
        return false;

    SDL_Rect DestR;

    DestR.x = X;
    DestR.y = Y;

    if(W > 0 && H > 0)
    {
        SDL_Rect SrcR;

        SrcR.x = 0;
        SrcR.y = 0;
        SrcR.w = W;
        SrcR.h = H;

        SDL_BlitSurface(Surf_Src, &SrcR, Surf_Dest, &DestR);
    }
    else
        SDL_BlitSurface(Surf_Src, NULL, Surf_Dest, &DestR);

    return true;
}

//Draw text to surface
bool Surface::DrawFont(SDL_Surface* Surf_Dest, int X, int Y, TTF_Font* font, const char* format, ...)
{
    if(Surf_Dest == NULL)
        return false;

    va_list argList;
    va_start(argList,format);
    char buf[512];
    vsnprintf(buf,512,format,argList);
    va_end(argList);

    SDL_Color foregroundColor = defcolor;

    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, buf, foregroundColor);

    SDL_Rect DestR;

    DestR.x = X;
    DestR.y = Y;

    SDL_BlitSurface(textSurface, NULL, Surf_Dest, &DestR);

    return true;
}

//Draw rectangle to surface
bool Surface::DrawRect(SDL_Surface* Surf_Dest, int X, int Y, int W, int H, Uint32 color)
{
    if(Surf_Dest == NULL)
        return false;

    SDL_Rect DestR;
    DestR.x=X;
    DestR.y=Y;
    DestR.w=W;
    DestR.h=H;
    SDL_FillRect(Surf_Dest,&DestR,color);

    return true;
}

