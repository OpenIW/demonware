// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdAuthTicket::bdAuthTicket()
    : m_magicNumber(0), m_type(0), m_titleID(0), m_timeIssued(0), m_timeExpires(0), m_licenseID(0), m_userID(0)
{
    bdMemset(m_username, 0, sizeof(m_username));
    bdMemset(m_sessionKey, 0, sizeof(m_sessionKey));
    m_usingHashMagicNumber[0] = 85;
    m_usingHashMagicNumber[1] = 51;
    m_usingHashMagicNumber[2] = 34;
    bdMemset(m_hash, 0, sizeof(m_hash));
}

void bdAuthTicket::deserialize(const void* buffer)
{
    bdUInt bufferSize = 128;
    bdUInt offset = 0;

    bdBytePacker::removeBasicType<bdUInt>(buffer, bufferSize, 0, offset, m_magicNumber);
    bdBytePacker::removeBasicType<bdUByte8>(buffer, bufferSize, offset, offset, m_type);
    bdBytePacker::removeBasicType<bdUInt>(buffer, bufferSize, offset, offset, m_titleID);
    bdBytePacker::removeBasicType<bdUInt>(buffer, bufferSize, offset, offset, m_timeIssued);
    bdBytePacker::removeBasicType<bdUInt>(buffer, bufferSize, offset, offset, m_timeExpires);
    bdBytePacker::removeBasicType<bdUInt64>(buffer, bufferSize, offset, offset, m_licenseID);
    bdBytePacker::removeBasicType<bdUInt64>(buffer, bufferSize, offset, offset, m_userID);
    bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(buffer), bufferSize, offset, offset, m_username, sizeof(m_username));
    bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(buffer), bufferSize, offset, offset, m_sessionKey, sizeof(m_sessionKey));
    bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(buffer), bufferSize, offset, offset, m_usingHashMagicNumber, sizeof(m_usingHashMagicNumber));
    bdBytePacker::removeBuffer(reinterpret_cast<const bdUByte8*>(buffer), bufferSize, offset, offset, m_hash, sizeof(m_hash));
}

void bdAuthTicket::serialize(void* buffer)
{
}
