// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdPerformanceValue : public bdTaskResult
{
public:
    bdUInt64 m_entityID;
    bdInt64 m_performanceValue;

    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdPerformanceValue();
    ~bdPerformanceValue();
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
    virtual void serialize(bdByteBuffer& buffer);
};
