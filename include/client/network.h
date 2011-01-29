#ifndef __NETWORK_H_
#define __NETWORK_H_

#include <global.h>
#include <../shared.h>

//Default server address and port
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 555

//maximal length of packet (think it will never reach 1KB, but who cares..)
#define MAXLEN (1*10*1024) // 10 KB

//default Network class
class Network
{
public:
    Network();

    void DoConnect(string phost = DEFAULT_IP, unsigned int pport = DEFAULT_PORT);
    bool IsConnected();
private:
    bool connected;
    IPaddress ip;
    TCPsocket sock;
    Uint16 port;
    char *name;
};

extern SDL_Thread *net_thread;

extern int SendToServer(GamePacket* packet);

#endif
