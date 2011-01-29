#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include <SDL.h>
#include <SDL_main.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <winsock.h>
#include <SDL_net.h>

#include <string.h>
#include <iostream>
using namespace std;
#include <time.h>

#include <interface.h>
#include <network.h>
#include <../shared.h>

//disable some stupid warnings
#pragma warning(disable:4996)

class Interface;
class Network;

//main application class
class Application
{
public:
    Application() {};

    void SetInterface(Interface* iface) { pInterface = iface; };

    bool OnInit();
    int  OnExecute();
    void OnEvent(SDL_Event* Event);

    SDL_Surface* GetDisplay() { return DrawSurface; };
    Interface* GetInterface() { return pInterface; };
private:
    SDL_Surface* DrawSurface;
    Interface* pInterface;
    bool Running;
};

extern Application Piskvorky;

extern void SetColor(unsigned int r, unsigned int g, unsigned int b);

//drawing helpers
class Surface
{
public:
    Surface();

    static SDL_Surface* Load(const char* File);
    static bool Draw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y, int W = 0, int H = 0);
    static bool DrawFont(SDL_Surface* Surf_Dest, int X, int Y, TTF_Font* font, const char* format, ...);
    static bool DrawRect(SDL_Surface* Surf_Dest, int X, int Y, int W, int H, Uint32 color);
};

#endif
