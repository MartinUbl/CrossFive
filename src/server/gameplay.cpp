#include <gameplay.h>

GamePlayHandler::GamePlayHandler()
{
    gamepair = new TGamePair[2];
    for(int i = 0; i < 2; i++)
    {
        gamepair[i].member = NULL;
        gamepair[i].marker = 0;
        gamepair[i].isTurn = false;
    }

    Game.inProgress = false;
    for(int i = 0; i < 40; i++)
        for(int j = 0; j < 40; j++)
            Game.field[i][j] = EMPTY;
}

GamePlayHandler::~GamePlayHandler()
{
    if(gamepair)
    {
        for(int i = 0; i < 2; i++)
            if(gamepair[i].member)
                delete gamepair[i].member;

        delete gamepair;
    }
}

int GamePlayHandler::SendPacket(TCPsocket sock, GamePacket* packet)
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

    packet->SetPos(0);
    for(size_t i = 0; i < packet->GetSize(); i++)
    {
        *packet >> buff[8+i];
    }

    char* buf = buff;
    sprintf(buf,"%s",buff);

    if(!buf || !psize)
        return 0;

    int len, result;

	len = psize + 1;

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

void GamePlayHandler::SendGlobalPacket(GamePacket* packet, Client* pExcept)
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

    packet->SetPos(0);
    for(size_t i = 0; i < packet->GetSize(); i++)
    {
        *packet >> buff[8+i];
    }

    char* buf = buff;
    sprintf(buf,"%s",buff);

    if(!buf || !psize)
        return;

    int len, result;

	len = psize+1;

	int cindex = 0;
	while(cindex < num_clients)
	{
        if(pExcept && clients[cindex].sock == pExcept->sock)
        {
            cindex++;
            continue;
        }

        len = SDL_SwapBE32(len);

        result = SDLNet_TCP_Send(clients[cindex].sock,&len,sizeof(len));
        if(result < sizeof(len))
        {
            if(SDLNet_GetError() && strlen(SDLNet_GetError()))
                printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
            remove_client(cindex);
            continue;
        }

        len = SDL_SwapBE32(len);

        result = SDLNet_TCP_Send(clients[cindex].sock,buf,len);
        if(result < len)
        {
            if(SDLNet_GetError() && strlen(SDLNet_GetError()))
                printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
            remove_client(cindex);
            continue;
        }

        cindex++;
    }
}

void GamePlayHandler::HandlePacket(GamePacket* packet, Client* pClient)
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
                if(!strcmp(versionstr,VERSION_STR))
                    data << (uint8)OK;
                else
                    data << (uint8)FAIL;
                data << uint32(strlen(name));
                data << name;
                data << uint32(pClient->guid);
                SendPacket(pClient->sock, &data);
            }
            break;
        case CMSG_HELLO:
            {
                GamePacket data(SMSG_PLAYER_JOINED);
                data << uint32(POS_OPONNENT);
                data << uint32(strlen(pClient->name));
                data << pClient->name;
                SendGlobalPacket(&data,pClient);

                //fake packet --> debug
                GamePacket data2(SMSG_PLAYER_JOINED);
                data2 << uint32(POS_OPONNENT);
                data2 << uint32(4);
                data2 << "abcd";
                SendPacket(pClient->sock, &data2);
            }
            break;
        case CMSG_READY_FOR_GAME:
            {
                //fake packet --> debug
                GamePacket data(SMSG_GAME_START);
                data << pClient->guid;
                data << uint8(0);
                data << uint32(888);
                data << uint8(1);
                SendGlobalPacket(&data);

                gamepair[0].isTurn = true;

                GamePacket data2(SMSG_SET_TURN);
                data2 << pClient->guid;
                SendGlobalPacket(&data2);

                return; //remove after debug done

                //Pokud uz je nejaky member ready
                if(gamepair[0].member && !gamepair[1].member)
                {
                    gamepair[1].member = pClient;
                    gamepair[1].marker = 1;

                    GamePacket data(SMSG_GAME_START);
                    data << gamepair[0].member->guid;
                    data << gamepair[0].marker;
                    data << gamepair[1].member->guid;
                    data << gamepair[1].marker;
                    SendGlobalPacket(&data);

                    gamepair[0].isTurn = true;

                    GamePacket data2(SMSG_SET_TURN);
                    data2 << gamepair[0].member->guid;
                    SendGlobalPacket(&data2);
                }
                //specialni pripad odpojeni klienta
                else if(!gamepair[0].member && gamepair[1].member)
                {
                    gamepair[0].member = pClient;
                    gamepair[0].marker = 0;

                    GamePacket data(SMSG_GAME_START);
                    data << gamepair[0].member->guid;
                    data << gamepair[0].marker;
                    data << gamepair[1].member->guid;
                    data << gamepair[1].marker;
                    SendGlobalPacket(&data);

                    gamepair[0].isTurn = true;

                    GamePacket data2(SMSG_SET_TURN);
                    data2 << gamepair[0].member->guid;
                    SendGlobalPacket(&data2);
                }
                //nikdo neni ready
                else if(!gamepair[0].member && !gamepair[1].member)
                {
                    gamepair[0].member = pClient;
                    gamepair[0].marker = 0;
                }
            }
            break;
        case CMSG_TURN:
            {
                uint8 field_x, field_y, symbol;

                *packet >> field_x >> field_y;

                uint32 clguid = pClient->guid;
                //odkomentovat po debugu
                /*if(gamepair[0].member->guid == clguid)
                    symbol = gamepair[0].marker;
                else if(gamepair[1].member->guid == clguid)
                    symbol = gamepair[1].marker;
                else
                    return;*/
                symbol = 1;

                if(field_x > 40 || field_y > 40)
                    return;

                //TODO: overit, zdali je tah platny

                Game.field[field_x][field_y] = symbol+1; //posun v enum o 1

                GamePacket data(SMSG_TURN);
                data << field_x;
                data << field_y;
                data << symbol;
                SendGlobalPacket(&data);
            }
            break;
    }
}

void GamePlayHandler::ProcessPacket(const char* message, Client* pClient)
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
