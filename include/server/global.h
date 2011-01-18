#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_net.h>

#include <../shared.h>

#pragma warning(disable:4996)

typedef struct {
	TCPsocket sock;
	char *name;
} Client;

extern char *getMsg(TCPsocket sock, char **buf);
extern int putMsg(TCPsocket sock, char *buf);
extern char *mformat(char *format,...);
extern void send_all(char *buf);
extern char *strsep(char **stringp, const char *delim);

extern void ProcessPacket(char* message, Client* pClient);

#endif
