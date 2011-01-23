#ifndef __GAMEPLAY_H_
#define __GAMEPLAY_H_

#include <global.h>

typedef struct
{
    Client* member;
    uint8 marker; //0 krizky, 1 kolecka
    bool isTurn; //je na tahu?
} TGamePair;

typedef struct
{
    bool inProgress;
    uint8 field[40][40]; //herni pole 40x40
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
    DefGame Game;
};

#endif
