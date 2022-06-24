// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdTimeStamp : public bdTaskResult
{
public:
    bdUInt m_timeStamp;

    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdTimeStamp();
    ~bdTimeStamp();
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
};