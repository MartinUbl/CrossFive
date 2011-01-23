#ifndef __OPCODES_H_
#define __OPCODES_H_

#define FAIL 0
#define OK 1

enum Opcodes
{
    MSG_NONE                       = 0x00,
    CMSG_LOGIN                     = 0x01,
    SMSG_LOGIN_RESPONSE            = 0x02,
    CMSG_HELLO                     = 0x03,
    SMSG_PLAYER_JOINED             = 0x04,
    CMSG_READY_FOR_GAME            = 0x05,
    SMSG_GAME_START                = 0x06,
    SMSG_SET_TURN                  = 0x07,
    CMSG_TURN                      = 0x08,
    SMSG_TURN                      = 0x09,
    SMSG_INVALID_TURN              = 0x0A,
    SMSG_WIN                       = 0x0B,
};

typedef enum
{
    POS_PLAYER,
    POS_OPONNENT,
    POS_SPECTATOR,
} ClientPositions;

#endif
