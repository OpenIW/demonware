// SPDX-License-Identifier: GPL-3.0-or-later

class bdPacketInterceptor
{
public:
    void operator delete(void* p);
    virtual ~bdPacketInterceptor() {};
    virtual bdBool acceptPacket(bdSocket*, bdAddr, void*, const bdUInt, const bdUByte8) { return false; };
};