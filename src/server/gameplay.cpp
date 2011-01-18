#include <global.h>

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

	len=psize+1;

	len=SDL_SwapBE32(len);

	result=SDLNet_TCP_Send(sock,&len,sizeof(len));
	if(result<sizeof(len)) {
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		return(0);
	}

	len=SDL_SwapBE32(len);
	
	result=SDLNet_TCP_Send(sock,buf,len);
	if(result<len) {
		if(SDLNet_GetError() && strlen(SDLNet_GetError()))
			printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		return(0);
	}

    return(result);
}

void HandlePacket(GamePacket* packet, Client* pClient)
{
    switch(packet->GetOpcode())
    {
        case CMSG_LOGIN:
            {
                unsigned int vsize,nsize;
                *packet >> vsize;
                const char* versionstr = packet->readstr(vsize);
                printf("Client version: %s\n",versionstr);
                *packet >> nsize;
                const char* name = packet->readstr(nsize);
                printf("Name: %s\n",name);
                pClient->name = (char*)name;

                GamePacket data(SMSG_LOGIN_RESPONSE);
                data << (unsigned int)4;
                data << "ABCD";
                SendPacket(pClient->sock, &data);
                break;
            }
        case CMSG_HELLO:
            {
                GamePacket data(SMSG_PLAYER_JOINED);
                data << uint32(4);
                data << "ahoj";
                SendPacket(pClient->sock, &data);
            }
            break;
    }
}

void ProcessPacket(char* message, Client* pClient)
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

    HandlePacket(&packet, pClient);
}
