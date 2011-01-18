#ifndef __OPCODES_H_
#define __OPCODES_H_

enum Opcodes
{
    MSG_NONE                       = 0x00,
    CMSG_LOGIN                     = 0x01,
    SMSG_LOGIN_RESPONSE            = 0x02,
    CMSG_HELLO                     = 0x03,
    SMSG_PLAYER_JOINED             = 0x04,
};

#endif
