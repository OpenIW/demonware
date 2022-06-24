// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdCounterValue : public bdTaskResult
{
public:
    bdUInt32 m_counterID;
    bdInt64 m_counterValue;

    bdCounterValue();
    virtual bdUInt sizeOf();
    virtual void serialize(bdByteBuffer& buffer);
    virtual bdBool deserialize(bdByteBufferRef buffer);
};
