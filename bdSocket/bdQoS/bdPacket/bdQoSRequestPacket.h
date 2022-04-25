// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdQoSRequestPacket
{
protected:
    bdUByte8 m_type;
    bdUInt64 m_timestamp;
    bdUInt32 m_id;
    bdUInt32 m_secid;
public:
    bdQoSRequestPacket();
    bdQoSRequestPacket(bdUInt32 secid, bdUInt32 id);
    bdUInt32 getId();
    bdUInt64 getTimestamp();
    bdUByte8 getType();
    bdUInt32 getSecId();
    bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    bdBool serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
};