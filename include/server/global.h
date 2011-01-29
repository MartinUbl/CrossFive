#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include <string.h>
#ifndef _WIN32
 #include <string>
 #include <SDL/SDL.h>
 #include <SDL/SDL_net.h>
#else
 #include <SDL.h>
 #include <SDL_net.h>
#endif
#include <stdarg.h>
#include <stdlib.h>

#include <../shared.h>

#pragma warning(disable:4996)

//default struct for clients
typedef struct {
    uint32 guid;
    TCPsocket sock;
    char* name;
} Client;

#include <gameplay.h>

//main.cpp - functions
extern char *getMsg(TCPsocket sock, char **buf);
extern int putMsg(TCPsocket sock, char *buf);
extern char *mformat(char *format,...);
extern void send_all(char *buf);
extern char *strsep(char **stringp, const char *delim);
extern void remove_client(int i);

//main.cpp - variables
extern Client *clients;
extern int num_clients;

#endif
