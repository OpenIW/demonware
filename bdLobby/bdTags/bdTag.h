// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdTag : public bdTaskResult
{
public:
    bdUInt64 m_priTag;
    bdUInt64 m_secTag;

    bdTag(bdTag* other);
    bdTag(bdUInt64 priTag, bdUInt64 secTag);
    bdTag();
    ~bdTag();
    virtual void serialize(bdByteBuffer* buffer) const;
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
    void set(bdUInt64 priTag, bdUInt64 secTag);
};