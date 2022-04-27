// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdDTLSInit : public bdDTLSHeader
{
protected:
    bdUInt16 m_initTag;
    bdSecurityID m_secID;
public:
    bdDTLSInit();
    bdDTLSInit(unsigned short initTag, bdSecurityID* secID);
    bdUInt16 getInitTag();
    void getSecID(bdSecurityID* secID);
    virtual bdBool serialize(void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
    virtual bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset);
};
