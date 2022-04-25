// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdQoSReplyPacket
{
protected:
    bdUByte8 m_type;
    bdUInt32 m_id;
    bdUInt64 m_timestamp;
    bdBool m_enabledMode;
    bdUByte8* m_data;
    bdUInt32 m_dataSize;
    bdUInt m_serializedSize;
public:
    static bdUInt headerSize;

    bdQoSReplyPacket();
    ~bdQoSReplyPacket();
    bdUByte8 getType();
    bdUInt32 getId();
    bdUInt64 getTimestamp();
    bdBool getEnabled();
    bdUByte8* getData();
    bdUInt32 getDataSize();
    bdUInt getSerializedSize();
    bdUInt getHeaderSize();
    bdBool setData(const bdUByte8* data, bdUInt32 dataSize);
    void setEnabled(bdBool mode);
    void setId(bdUInt32 id);
    void setTimestamp(bdUInt64 timestamp);
    bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    bdBool serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
};