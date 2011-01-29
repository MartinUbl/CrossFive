#include <gameplay.h>

//default constructor
GamePlayHandler::GamePlayHandler()
{
    //Initialize and clear gamepair array
    gamepair = new TGamePair[2];
    for(int i = 0; i < 2; i++)
    {
        gamepair[i].member = NULL;
        gamepair[i].marker = 0;
        gamepair[i].isTurn = false;
        gamepair[i].present = false;
    }

    //clear game field
    Game.inProgress = false;
    for(int i = 0; i < 40; i++)
        for(int j = 0; j < 40; j++)
            Game.field[i][j] = EMPTY;
}

//default destructor
GamePlayHandler::~GamePlayHandler()
{
    //only free some memory
    if(gamepair)
    {
        for(int i = 0; i < 2; i++)
            if(gamepair[i].member)
                delete gamepair[i].member;

        delete gamepair;
    }
}

//Function for sending packet
int GamePlayHandler::SendPacket(TCPsocket sock, GamePacket* packet)
{
    //total size + 4 bytes for opcode ID + 4 bytes for packet body size
    size_t psize = packet->GetSize() + 4 + 4;

    char* buff = new char[psize];
    //at first, add opcode ID parsed to bytes
    buff[0] = HIPART32(packet->GetOpcode())/0x100;
    buff[1] = HIPART32(packet->GetOpcode())%0x100;
    buff[2] = LOPART32(packet->GetOpcode())/0x100;
    buff[3] = LOPART32(packet->GetOpcode())%0x100;

    //add size parsed to bytes
    buff[4] = HIPART32(packet->GetSize())/0x100;
    buff[5] = HIPART32(packet->GetSize())%0x100;
    buff[6] = LOPART32(packet->GetSize())/0x100;
    buff[7] = LOPART32(packet->GetSize())%0x100;

    //add all body data as bytes
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

//Function for sending packets globally
void GamePlayHandler::SendGlobalPacket(GamePacket* packet, Client* pExcept)
{
    //total size + 4 bytes for opcode ID + 4 bytes for packet body size
    size_t psize = packet->GetSize() + 4 + 4;

    char* buff = new char[psize];
    //at first, add opcode ID parsed to bytes
    buff[0] = HIPART32(packet->GetOpcode())/0x100;
    buff[1] = HIPART32(packet->GetOpcode())%0x100;
    buff[2] = LOPART32(packet->GetOpcode())/0x100;
    buff[3] = LOPART32(packet->GetOpcode())%0x100;

    //add size parsed to bytes
    buff[4] = HIPART32(packet->GetSize())/0x100;
    buff[5] = HIPART32(packet->GetSize())%0x100;
    buff[6] = LOPART32(packet->GetSize())/0x100;
    buff[7] = LOPART32(packet->GetSize())%0x100;

    //add all body data as bytes
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

//Handling for incoming packets
void GamePlayHandler::HandlePacket(GamePacket* packet, Client* pClient)
{
    switch(packet->GetOpcode())
    {
        case CMSG_LOGIN: //Login packet (New game press)
            {
                unsigned int vsize,nsize;
                *packet >> vsize;
                const char* versionstr = packet->readstr(vsize);
                *packet >> nsize;
                const char* name = packet->readstr(nsize);
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
        case CMSG_HELLO: //Hello packet (New game press, login success)
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
        case CMSG_READY_FOR_GAME: //Client is ready for game (and joining main game), so put him to queue
            {
                //fake packet --> debug
                GamePacket data(SMSG_GAME_START);
                data << pClient->guid;
                data << uint8(0);
                data << uint32(888);
                data << uint8(1);
                SendGlobalPacket(&data);

                gamepair[0].isTurn = true;
                gamepair[1].member = pClient;

                GamePacket data2(SMSG_SET_TURN);
                data2 << pClient->guid;
                SendGlobalPacket(&data2);

                return; //remove after debug done

                //If some member ready, add as second from pair
                if(gamepair[0].present && !gamepair[1].present)
                {
                    //add data to struct
                    gamepair[1].member = pClient;
                    gamepair[1].marker = 1;
                    gamepair[1].present = true;
                    gamepair[1].name = pClient->name;
                    gamepair[1].guid = pClient->guid;

                    //send global message, that a player has joined
                    GamePacket dataa(SMSG_PLAYER_JOINED);
                    dataa << uint32(POS_OPONNENT);
                    dataa << uint32(strlen(gamepair[0].name.c_str()));
                    dataa << gamepair[0].name.c_str();
                    SendPacket(pClient->sock,&dataa);

                    //and we are ready to start the game
                    GamePacket data(SMSG_GAME_START);
                    data << gamepair[0].guid;
                    data << gamepair[0].marker;
                    data << gamepair[1].guid;
                    data << gamepair[1].marker;
                    SendGlobalPacket(&data);

                    //set turn to first joined player and send set turn message
                    gamepair[0].isTurn = true;

                    GamePacket data2(SMSG_SET_TURN);
                    data2 << gamepair[0].guid;
                    SendGlobalPacket(&data2);
                }
                //special case - if client disconnects
                else if(!gamepair[0].present && gamepair[1].present)
                {
                    //add data to struct
                    gamepair[0].member = pClient;
                    gamepair[0].marker = 0;
                    gamepair[0].present = true;
                    gamepair[0].name = pClient->name;
                    gamepair[0].guid = pClient->guid;

                    //send global message, that a player has joined
                    GamePacket dataa(SMSG_PLAYER_JOINED);
                    dataa << uint32(POS_OPONNENT);
                    dataa << uint32(strlen(gamepair[1].name.c_str()));
                    dataa << gamepair[1].name.c_str();
                    SendPacket(pClient->sock,&dataa);

                    //and we are ready to start the game
                    GamePacket data(SMSG_GAME_START);
                    data << gamepair[0].guid;
                    data << gamepair[0].marker;
                    data << gamepair[1].guid;
                    data << gamepair[1].marker;
                    SendGlobalPacket(&data);

                    //set turn to first joined player and send set turn message
                    gamepair[0].isTurn = true;

                    GamePacket data2(SMSG_SET_TURN);
                    data2 << gamepair[0].guid;
                    SendGlobalPacket(&data2);
                }
                //nobody's ready
                else if(!gamepair[0].present && !gamepair[1].present)
                {
                    gamepair[0].member = pClient;
                    gamepair[0].marker = 0;
                    gamepair[0].present = true;
                    gamepair[0].name = pClient->name;
                    gamepair[0].guid = pClient->guid;
                }
            }
            break;
        case CMSG_TURN: //Turn packet
            {
                uint8 field_x, field_y, symbol;

                *packet >> field_x >> field_y;

                uint32 clguid = pClient->guid;

                symbol = 1;

                //debug - uncomment after debug done
                /*if(gamepair[0].member != NULL && gamepair[0].guid == clguid)
                    symbol = gamepair[0].marker+1;
                else if(gamepair[1].member != NULL && gamepair[1].guid == clguid)
                    symbol = gamepair[1].marker+1;
                else
                    return;

                if(gamepair[0].guid == clguid && !gamepair[0].isTurn)
                    return;
                if(gamepair[1].guid == clguid && !gamepair[1].isTurn)
                    return;*/

                //verify out of range
                if(field_x > 40 || field_y > 40)
                    return;

                //verify free field
                if(Game.field[field_x][field_y] != 0)
                {
                    GamePacket data(SMSG_INVALID_TURN);
                    data << field_x;
                    data << field_y;
                    SendPacket(pClient->sock,&data);

                    return;
                }

                //set field and let all clients know, that somebody turned
                Game.field[field_x][field_y] = symbol; //posun v enum o 1

                GamePacket data(SMSG_TURN);
                data << field_x;
                data << field_y;
                data << symbol;
                SendGlobalPacket(&data);

                //move turn to next player
                GamePacket data2(SMSG_SET_TURN);
                data2 << uint32((gamepair[0].guid == clguid)?gamepair[1].guid:gamepair[0].guid);
                SendGlobalPacket(&data2);

                gamepair[0].isTurn = !gamepair[0].isTurn;
                gamepair[1].isTurn = !gamepair[1].isTurn;
            }
            break;
    }
}

//Function for processing packets
void GamePlayHandler::ProcessPacket(const char* message, Client* pClient)
{
    unsigned int opcode, size;

    //at first, parse opcode id
    opcode =  message[0]*0x1000000;
    opcode += message[1]*0x10000;
    opcode += message[2]*0x100;
    opcode += message[3];

    GamePacket packet(opcode);

    //then parse size
    size =  message[4]*0x1000000;
    size += message[5]*0x10000;
    size += message[6]*0x100;
    size += message[7];

    //and process a packet body
    for(size_t i = 0; i < size; i++)
        packet << (unsigned char)message[8+i];

    //finally, let it handle
    HandlePacket(&packet, pClient);
}
