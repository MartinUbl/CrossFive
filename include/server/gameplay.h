#ifndef __GAMEPLAY_H_
#define __GAMEPLAY_H_

#include <global.h>

//gamepair structure
struct TGamePair
{
    bool present;
    Client* member;
    uint8 marker; //0 cross, 1 circles
    bool isTurn; //is on turn?
    std::string name;
    uint32 guid;
};

//game variables structure
struct DefGame
{
    bool inProgress;
    uint8 field[40][40]; //game field 40x40
    uint16 wincoord[4]; //winning coordinates 0,1-first x,y, 2,3-second x,y
};

//default gameplay handler class
class GamePlayHandler
{
public:
    GamePlayHandler();
    ~GamePlayHandler();

    int SendPacket(TCPsocket sock, GamePacket* packet);
    void SendGlobalPacket(GamePacket* packet, Client* pExcept = NULL);
    void HandlePacket(GamePacket* packet, Client* pClient);
    void ProcessPacket(const char* message, Client* pClient);

    unsigned int CheckWin();
    void Win(unsigned int symbol);
private:
    TGamePair* gamepair;
    DefGame Game;
};

#endif
