#ifndef __GAMEPLAY_H_
#define __GAMEPLAY_H_

#include <global.h>

struct TGamePair
{
    bool present;
    Client* member;
    uint8 marker; //0 krizky, 1 kolecka
    bool isTurn; //je na tahu?
    std::string name;
    uint32 guid;
};

struct DefGame
{
    bool inProgress;
    uint8 field[40][40]; //herni pole 40x40
};

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
