#include <global.h>
#include <windows.h>

int net_thread_main(void *);
int local_thread_main(void *);

TCPsocket serversock;

int done = 0;
SDL_Thread *net_thread = NULL;

char *getMsg(TCPsocket sock, char **buf)
{
	Uint32 len,result;
	static char *_buf;

	if(!buf)
		buf = &_buf;

	if(*buf)
		free(*buf);
	*buf = NULL;

	result = SDLNet_TCP_Recv(sock,&len,sizeof(len));
	if(result < sizeof(len))
	{
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Recv: %s\n", SDLNet_GetError());
		return NULL;
	}

	len=SDL_SwapBE32(len);

	if(!len)
		return NULL;

	*buf = (char*)malloc(len);
	if(!(*buf))
		return NULL;

	result = SDLNet_TCP_Recv(sock,*buf,len);
	if(result < len)
	{
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Recv: %s\n", SDLNet_GetError());
		free(*buf);
		buf = NULL;
	}

	return (*buf);
}

int putMsg(TCPsocket sock, char *buf)
{
	Uint32 len,result;

	if(!buf || !strlen(buf))
		return 1;

	len = strlen(buf)+1;

	len = SDL_SwapBE32(len);

	result = SDLNet_TCP_Send(sock,&len,sizeof(len));
	if(result < sizeof(len))
    {
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		return 0;
	}

	len = SDL_SwapBE32(len);

	result = SDLNet_TCP_Send(sock,buf,len);
	if(result < len)
    {
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		return 0;
	}

	return result;
}

char *strsep(char **stringp, const char *delim)
{
	char *p;
	
	if(!stringp)
		return NULL;

	p = *stringp;
	while(**stringp && !strchr(delim,**stringp))
		(*stringp)++;

	if(**stringp)
	{
		**stringp = '\0';
		(*stringp)++;
	}
	else
		*stringp = NULL;

	return p;
}

int SendPacket(TCPsocket sock, GamePacket* packet)
{
    //celkova velikost + 4 bajty na ID opkodu + 4 bajty na velikost tela packetu
    size_t psize = packet->GetSize() + 4 + 4;

    char* buff = new char[psize];
    buff[0] = HIPART32(packet->GetOpcode())/0x100;
    buff[1] = HIPART32(packet->GetOpcode())%0x100;
    buff[2] = LOPART32(packet->GetOpcode())/0x100;
    buff[3] = LOPART32(packet->GetOpcode())%0x100;

    buff[4] = HIPART32(packet->GetSize())/0x100;
    buff[5] = HIPART32(packet->GetSize())%0x100;
    buff[6] = LOPART32(packet->GetSize())/0x100;
    buff[7] = LOPART32(packet->GetSize())%0x100;

    for(size_t i = 0; i < packet->GetSize(); i++)
    {
        *packet >> buff[8+i];
    }

    char* buf = buff;
    sprintf(buf,"%s",buff);

    if(!buf || !psize)
        return 0;

    int len, result;

	len = psize+1;

	len = SDL_SwapBE32(len);

	result = SDLNet_TCP_Send(sock,&len,sizeof(len));
	if(result < sizeof(len))
    {
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		return 0;
	}

	len = SDL_SwapBE32(len);
	
	result = SDLNet_TCP_Send(sock,buf,len);
	if(result < len)
    {
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		return 0;
	}

    return result;
}

int SendToServer(GamePacket* packet)
{
    return SendPacket(serversock,packet);
}

Network::Network()
{
    connected = false;
}

bool Network::IsConnected()
{
    return connected;
}

void Network::DoConnect(std::string phost, unsigned int pport)
{
    if(SDLNet_Init() == -1)
    {
        //Nepodarilo se pripojit
        connected = false;
        return;
    }

    port = pport;

    if(SDLNet_ResolveHost(&ip,phost.c_str(),port) == -1)
	{
		connected = false;
		SDLNet_Quit();
        return;
	}

    sock = SDLNet_TCP_Open(&ip);

    if(!sock)
    {
        connected = false;
        return;
    }

    char* name = new char[5];
    for(int i = 0; i < 4; i++)
        name[i] = 70+rand()%20;
    name[4] = '\0';

    if(!putMsg(sock,(char*)name))
	{
        connected = false;
		SDLNet_TCP_Close(sock);
		SDLNet_Quit();
		SDL_Quit();
        exit(0);
		return;
	}

    serversock = sock;

    GamePacket data(CMSG_LOGIN);
    data << uint32(strlen(VERSION_STR));
    data << VERSION_STR;
    data << uint32(strlen(name));
    data << name;
    SendPacket(sock,&data);

    net_thread = SDL_CreateThread(net_thread_main,sock);

    //SDL_WaitThread(local_thread,NULL);
	//SDL_WaitThread(net_thread,NULL);
}

void HandlePacket(GamePacket* packet, TCPsocket sock)
{
    Interface* pIf = Piskvorky.GetInterface();

    switch(packet->GetOpcode())
    {
        case SMSG_LOGIN_RESPONSE:
            {
                uint8 loginstate;
                uint32 nsize, sguid;

                *packet >> loginstate;
                *packet >> nsize;
                const char* name = packet->readstr(nsize);
                *packet >> sguid;

                if(loginstate != OK)
                {
                    //Login failed, return
                    return;
                }

                gStore.SetMyGUID(sguid);
                gStore.SetName(name);

                GamePacket data(CMSG_HELLO);
                data << uint32(0);
                SendPacket(sock,&data);

                pIf->SetStage(STAGE_CONNECTING);
                pIf->StoreChanged();

                GamePacket data2(CMSG_READY_FOR_GAME);
                data2 << uint32(1);
                SendPacket(sock,&data2);

                break;
            }
        case SMSG_PLAYER_JOINED:
            {
                if(pIf->GetStage() == STAGE_CONNECTING)
                {
                    uint32 pos,namelen;
                    *packet >> pos;
                    *packet >> namelen;
                    const char* pname = packet->readstr(namelen);

                    if(pos == POS_OPONNENT)
                    {
                        gStore.SetOponnentName(pname);
                        pIf->SetStage(STAGE_GAME);
                    }
                }
                break;
            }
        case SMSG_GAME_START:
            {
                //TODO: handle allowing to play
            }
            break;
        case SMSG_SET_TURN:
            {
                //TODO: implement
            }
            break;
        case SMSG_TURN:
            {
                unsigned char field_x, field_y, symbol;
                *packet >> field_x >> field_y >> symbol;

                gStore.SetFieldValue(field_x, field_y, symbol);
                pIf->StoreChanged();
            }
            break;
        default:
            MessageBox(0,"Prijat neznamy opkod","Chyba",0);
            break;
    }
}

void ProcessPacket(char* message, TCPsocket sock)
{
    unsigned int opcode, size;

    opcode =  message[0]*0x1000000;
    opcode += message[1]*0x10000;
    opcode += message[2]*0x100;
    opcode += message[3];

    GamePacket packet(opcode);

    size =  message[4]*0x1000000;
    size += message[5]*0x10000;
    size += message[6]*0x100;
    size += message[7];

    for(size_t i = 0; i < size; i++)
        packet << (unsigned char)message[8+i];

    HandlePacket(&packet, sock);
}

int net_thread_main(void *data)
{
	TCPsocket sock = (TCPsocket)data;
	SDLNet_SocketSet set;
	int numready;
	char *str = NULL;

	set = SDLNet_AllocSocketSet(1);
	if(!done && !set)
	{
		printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
		SDLNet_Quit();
		SDL_Quit();
		done = 2;
	}

	if(!done && SDLNet_TCP_AddSocket(set,sock) == -1)
	{
		printf("SDLNet_TCP_AddSocket: %s\n",SDLNet_GetError());
		SDLNet_Quit();
		SDL_Quit();
		done = 3;
	}
	
	while(!done)
	{
		numready = SDLNet_CheckSockets(set, (Uint32)-1);
		if(numready == -1)
		{
			printf("SDLNet_CheckSockets: %s\n",SDLNet_GetError());
			done = 4;
			break;
		}

		if(numready && SDLNet_SocketReady(sock))
		{
			if(!getMsg(sock,&str))
			{
				char *errstr=SDLNet_GetError();
				printf("getMsg: %s\n",strlen(errstr)?errstr:"Server disconnected");
				done = 5;
				break;
			}
            ProcessPacket(str, sock);
		}
	}

	if(!done)
		done = 1;

	return 0;
}
