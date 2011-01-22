#ifndef __SHARED_H_
#define __SHARED_H_

#include <vector>

#include "opcodes.h"

#define VERSION_STR "v1b"

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

#define HIPART32(a) a/0x10000
#define LOPART32(a) a%0x10000

class GamePacket
{
public:
    GamePacket()
    {
        opcodeId = MSG_NONE;
        storage.clear();
        size = 0;
        pos = 0;
    }
    GamePacket(unsigned int opcode)
    {
        opcodeId = opcode;
        storage.clear();
        size = 0;
        pos = 0;
    }

    void SetOpcode(unsigned int opcode) { opcodeId = opcode; }
    unsigned int GetOpcode() { return opcodeId; }

    void SetPos(size_t newpos) { pos = newpos; }
    unsigned int GetPos() { return pos; }

    size_t GetSize() { return size; }

    GamePacket &operator<<(unsigned char value)
    {
        storage.resize(size + 1);
        storage[size] = value;
        size += 1;
        return *this;
    }
    GamePacket &operator<<(unsigned short value)
    {
        storage.resize(size + 2);
        storage[size]   = value/0x100;
        storage[size+1] = value%0x100;
        size += 2;
        return *this;
    }
    GamePacket &operator<<(unsigned int value)
    {
        unsigned short hipart,lopart;
        hipart = value/0x10000;
        lopart = value%0x10000;
        storage.resize(size + 4);
        storage[size]   = (hipart/0x100);
        storage[size+1] = (hipart%0x100);
        storage[size+2] = (lopart/0x100);
        storage[size+3] = (lopart%0x100);
        size += 4;
        return *this;
    }
    GamePacket &operator<<(const char* value)
    {
        for(size_t i = 0; i < strlen(value); i++)
        {
            storage.resize(size + 1);
            storage[size]   = (value[i]);
            size += 1;
        }
        return *this;
    }

    GamePacket &operator>>(char &value)
    {
        value = storage[pos];
        pos += 1;
        return *this;
    }
    GamePacket &operator>>(unsigned char &value)
    {
        value = storage[pos];
        pos += 1;
        return *this;
    }
    GamePacket &operator>>(unsigned short &value)
    {
        value  = storage[pos]  *0x100;
        value += storage[pos+1];
        pos += 2;
        return *this;
    }
    GamePacket &operator>>(unsigned int &value)
    {
        value  = storage[pos]  *0x1000000;
        value += storage[pos+1]*0x10000;
        value += storage[pos+2]*0x100;
        value += storage[pos+3];
        pos += 4;
        return *this;
    }
    const char* readstr(size_t size)
    {
        char* temp = new char[size+1];
        for(size_t i = 0; i < size; i++)
        {
            temp[i] = storage[pos];
            pos += 1;
        }
        temp[size] = '\0';

        return temp;
    }

private:
    unsigned int opcodeId;
    std::vector<unsigned char> storage;
    size_t size, pos;
};

#endif
