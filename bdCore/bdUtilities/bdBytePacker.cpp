// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

bdBool bdBytePacker::appendBuffer(bdUByte8* dest, bdUInt destSize, bdUInt offset, bdUInt& newOffset, const void* src, bdUInt writeSize)
{
    newOffset = writeSize + offset;

    if (dest && src)
    {
        bdAssert(offset <= destSize, "Offset is past the end of the destination buffer.");
        if (newOffset > destSize)
        {
            bdLogWarn("byte packer", "Not enough room left to write %u bytes.", writeSize);
        }

        if (offset <= destSize && newOffset <= destSize)
        {
            bdMemmove(&dest[offset], src, writeSize);
            return true;
        }
        return false;
    }
    return true;
}

bdBool bdBytePacker::appendEncodedUInt16(bdUByte8* buffer, bdUInt bufferSize, bdUInt offset, bdUInt& newOffset, bdUInt16 value)
{
    if (value <= 0x7F)
    {
        return bdBytePacker::appendBasicType<bdUByte8>(buffer, bufferSize, offset, newOffset, value);
    }
    else
    {
        bdAssert(value < 0x8000u, "Integer is too big.");
        bdUByte8 src = value | 0x8000;
        bool appended = bdBytePacker::appendBasicType<bdUByte8>(buffer, bufferSize, offset, newOffset, src + 1);
        appended = appended == bdBytePacker::appendBasicType<bdUByte8>(buffer, bufferSize, newOffset, newOffset, src);
        return appended;
    }
}

bdBool bdBytePacker::removeBuffer(const bdUByte8* src, bdUInt srcSize, bdUInt offset, bdUInt& newOffset, void* const dest, bdUInt readSize)
{
    newOffset = readSize + offset;

    if (dest && src)
    {
        bdAssert(offset <= srcSize, "Offset is past the end of the source buffer.");
        if (newOffset > srcSize)
        {
            bdLogWarn("byte packer", "Not enough data left to read %u bytes.", readSize);
        }

        if ((newOffset <= srcSize) && offset <= srcSize)
        {
            bdMemmove(dest, &src[offset], readSize);
            return true;
        }
        return false;
    }
    return true;
}

bdBool bdBytePacker::removeEncodedUInt16(const bdUByte8* buffer, bdUInt bufferSize, bdUInt offset, bdUInt& newOffset, bdUInt16& value)
{
    bdUByte8 firstByte = 0;

    bdBool ok = removeBasicType<bdUByte8>(buffer, bufferSize, offset, newOffset, firstByte);

    if (firstByte < 0x80)
    {
        value = firstByte;
        return ok;
    }

    firstByte &= ~0x80;
    bdUByte8 secondByte = 0;
    ok = ok == bdBytePacker::removeBasicType<bdUByte8>(buffer, bufferSize, newOffset, newOffset, secondByte);
    value = secondByte + (firstByte << 8);
    return ok;
}

bdBool bdBytePacker::skipBytes(const bdUByte8* buffer, bdUInt bufferSize, bdUInt offset, bdUInt& newOffset, bdUInt bytes)
{
    newOffset = bytes + offset;

    if (buffer)
    {
        return bufferSize >= newOffset;
    }
    return false;
}

bdBool bdBytePacker::rewindBytes(const bdUByte8* buffer, bdUInt bufferSize, bdUInt offset, bdUInt& newOffset, bdUInt bytes)
{
    newOffset = offset - bytes;

    if (buffer)
    {
        return offset >= bytes;
    }
    return false;
}
