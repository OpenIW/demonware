// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSessionParams
{
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    virtual ~bdSessionParams();
    virtual bdBool serialize(bdByteBuffer* buffer);
    virtual bdUInt sizeOf();
};