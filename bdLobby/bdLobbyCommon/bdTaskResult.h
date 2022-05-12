// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdTaskResult
{
public:
    virtual ~bdTaskResult();
    virtual bdBool deserialize(bdByteBufferRef);
    virtual bdUInt sizeOf();
};