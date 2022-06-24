// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdBytePacker
{
public:
    static bdBool appendBuffer(bdUByte8* dest, bdUInt destSize, bdUInt offset, bdUInt& newOffset, const void* src, bdUInt writeSize);
    static bdBool appendEncodedUInt16(bdUByte8* buffer, bdUInt bufferSize, bdUInt offset, bdUInt& newOffset, bdUInt16 value);
    static bdBool removeBuffer(const bdUByte8* src, bdUInt srcSize, bdUInt offset, bdUInt& newOffset, void* const dest, bdUInt readSize);
    static bdBool removeEncodedUInt16(const bdUByte8* buffer, bdUInt bufferSize, bdUInt offset, bdUInt& newOffset, bdUInt16& value);
    static bdBool skipBytes(const bdUByte8* buffer, bdUInt bufferSize, bdUInt offset, bdUInt& newOffset, bdUInt bytes);
    static bdBool rewindBytes(const bdUByte8* buffer, bdUInt bufferSize, bdUInt offset, bdUInt& newOffset, bdUInt bytes);
    template <typename varType>
    static bdBool removeBasicType(const void* buffer, const bdUInt bufferSize, bdUInt offset, bdUInt& newOffset, varType& var);
    template <typename varType>
    static bdBool appendBasicType(void* buffer, const bdUInt bufferSize, bdUInt offset, bdUInt& newOffset, const varType& var);
};

template<typename varType>
bdBool bdBytePacker::removeBasicType(void const* buffer, const bdUInt bufferSize, bdUInt offset, bdUInt& newOffset, varType& var)
{
    bdBool read;
    varType nvar;

    read = removeBuffer(reinterpret_cast<const bdUByte8*>(buffer), bufferSize, offset, newOffset, &nvar, sizeof(varType));
    if (read)
    {
        buffer = &nvar;
        bdBitOperations::endianSwap<varType>(nvar, var);
    }
    return read;
}

template<typename varType>
bdBool bdBytePacker::appendBasicType(void* buffer, const bdUInt bufferSize, bdUInt offset, bdUInt& newOffset, const varType& var)
{
    varType nvar;

    bdBitOperations::endianSwap<varType>(var, nvar);
    return appendBuffer(reinterpret_cast<bdUByte8*>(buffer), bufferSize, offset, newOffset, &nvar, sizeof(varType));
}

// Keep this just in case.
#define SkipBytes(status, data, size, offset, newOffset, bytes)                                                \
if (status)                                                                                                    \
{                                                                                                              \
    status = bdBytePacker::skipBytes(reinterpret_cast<const bdUByte8*>(data), size, offset, newOffset, bytes); \
}

#define RewindBytes(status, data, size, offset, newOffset, bytes)                                                \
if (status)                                                                                                      \
{                                                                                                                \
    status = bdBytePacker::rewindBytes(reinterpret_cast<const bdUByte8*>(data), size, offset, newOffset, bytes); \
}

#define RemoveBasicType(status, type, data, size, offset, newOffset, var)                                            \
if (status)                                                                                                          \
{                                                                                                                    \
    status = bdBytePacker::removeBasicType<type>(reinterpret_cast<const void*>(data), size, offset, newOffset, var); \
}

#define AppendBasicType(status, type, data, size, offset, newOffset, var)                                            \
if (status)                                                                                                          \
{                                                                                                                    \
    status = bdBytePacker::appendBasicType<type>(reinterpret_cast<void*>(data), size, offset, newOffset, var); \
}

#define Deserialize(status, var, data, size, offset, newOffset)  \
if (status)                                                      \
{                                                                \
    status = var.deserialize(reinterpret_cast<const bdUByte8*>(data), size, offset, newOffset);      \
}

#define Serialize(status, var, data, size, offset, newOffset)    \
if (status)                                                      \
{                                                                \
    status = var.serialize(reinterpret_cast<bdUByte8*>(data), size, offset, newOffset);      \
}
