// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define BD_MAX_MESSAGE_SIZE 1236
typedef bdUByte8 bdMessageType;

class bdMessage : bdReferencable
{
protected:
    bdMessageType m_type;
    bdBitBufferRef m_payload;
    bdBool m_payloadTypeChecked;
    bdByteBufferRef m_unencPayload;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdMessage(const bdMessageType type, const bdBool typeChecked);
    bdMessage(const bdMessageType type, const bdUByte8* const payload, const bdUInt32 size, const bdBool hasTypeCheckedBit, const bdUByte8* const unencPayload, const bdUInt32 unencSize);
    void createUnencryptedPayload(const bdUInt size);
    bdBitBufferRef getPayload();
    const bdByteBufferRef getUnencryptedPayload() const;
    const bdBool hasUnencryptedPayload() const;
    const bdBool hasPayload() const;
    const bdMessageType getType() const;
};

typedef bdReference<bdMessage> bdMessageRef;
