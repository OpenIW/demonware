// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdDTLSHeader
{
protected:
    bdUByte8 m_type;
    bdUByte8 m_version;
    bdUInt16 m_vtag;
    bdUInt16 m_counter;
public:
    void operator delete(void* p);
    bdDTLSHeader();
    bdDTLSHeader(bdDTLSPacketTypes type, bdUInt16 vtag, bdUInt16 counter);
    virtual ~bdDTLSHeader();
    virtual bdBool serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    virtual bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    bdUByte8 getType();
    bdUByte8 getVersion();
    bdUInt16 getVtag();
    bdUInt16 getCounter();
};