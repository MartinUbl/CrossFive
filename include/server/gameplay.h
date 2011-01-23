#ifndef __GAMEPLAY_H_
#define __GAMEPLAY_H_

#include <global.h>

typedef struct
{
    Client* member;
    uint8 marker; //0 krizky, 1 kolecka
} TGamePair;

typedef struct
{
    bool inProgress;
    uint8 field[20][20]; //herni pole 20x20? Upravit podle klienta
} DefGame;

class GamePlayHandler
{
public:
    GamePlayHandler();
    ~GamePlayHandler();

    int SendPacket(TCPsocket sock, GamePacket* packet);
    void SendGlobalPacket(GamePacket* packet, Client* pExcept = NULL);
    void HandlePacket(GamePacket* packet, Client* pClient);
    void ProcessPacket(const char* message, Client* pClient);
private:
    TGamePair* gamepair;
};

#endif
