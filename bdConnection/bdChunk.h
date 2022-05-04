// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

enum bdChunkTypes : bdInt
{
    BD_CT_INVALID = 0x0,
    BD_CT_PADDING = 0x1,
    BD_CT_DATA = 0x2,
    BD_CT_INIT = 0x3,
    BD_CT_INIT_ACK = 0x4,
    BD_CT_SACK = 0x5,
    BD_CT_HEARTBEAT = 0x6,
    BD_CT_HEARTBEAT_ACK = 0x7,
    BD_CT_ABORT = 0x8,
    BD_CT_SHUTDOWN = 0x9,
    BD_CT_SHUTDOWN_ACK = 0xA,
    BD_CT_SHUTDOWN_COMPLETE = 0xB,
    BD_CT_ERROR = 0xC,
    BD_CT_COOKIE_ECHO = 0xD,
    BD_CT_COOKIE_ACK = 0xE,
};

class bdChunk : public bdReferencable
{
public:
    struct bdChunkData
    {
        bdUByte8 m_type;
        bdUByte8 m_flags;
        bdUInt16 m_length;
    };
protected:
    bdChunkTypes m_type;
    bdChunk::bdChunkData m_headerData;
public:
    bdChunk(const bdChunkTypes type) : bdReferencable(), m_type(type) {};
    virtual bdUInt serialize(bdUByte8* data, const bdUInt32 size);
    virtual bdBool deserialize(const bdUByte8* const data, const bdUInt size, bdUInt* offset);
    virtual bdUInt getSerializedSize();
    const bdChunkTypes getType() const;
    const bdBool isControl() const;

    static bdChunkTypes getType(const bdUByte8* const data, const bdUInt size);
};

typedef bdReference<bdChunk> bdChunkRef;
