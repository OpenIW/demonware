// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdByteBuffer::bdByteBuffer() : bdReferencable()
{
}

bdByteBuffer::bdByteBuffer(void* bytes, const bdUInt size, bdBool isTypeChecked) : bdReferencable()
{
    m_size = size;
    m_data = NULL;
    m_typeChecked = isTypeChecked;
    m_typeCheckedCopy = m_typeChecked;
    m_allocatedData = 0;
    if (size)
    {
        m_data = bdAllocate<bdUByte8>(size);
        m_allocatedData = true;
        bdMemcpy(m_data, bytes, size);
    }
    m_writePtr = m_data;
    m_readPtr = m_data;
}

bdByteBuffer::bdByteBuffer(const bdUInt size, bdBool isTypeChecked) : bdReferencable()
{
    m_size = size;
    m_data = NULL;
    m_typeChecked = isTypeChecked;
    m_typeCheckedCopy = m_typeChecked;
    m_allocatedData = false;
    if (size)
    {
        m_data = bdAllocate<bdUByte8>(size);
        m_allocatedData = true;
    }
    m_writePtr = m_data;
    m_readPtr = m_data;
}

bdByteBuffer::~bdByteBuffer()
{
    m_allocatedData = true;
    bdAssert(m_data == BD_NULL || m_allocatedData, "Potential deallocation error with derived class");
    if (m_data && m_allocatedData)
    {
        bdDeallocate<bdUByte8>(m_data);
    }
    m_data = NULL;
    m_readPtr = NULL;
    m_writePtr = NULL;
}

bdUInt bdByteBuffer::getDataSize()
{
    return (bdInt32)m_writePtr - (bdInt32)m_data;
}

bdUInt bdByteBuffer::getReadSize()
{
    return (bdInt32)m_readPtr - (bdInt32)m_data;
}

bdBool bdByteBuffer::getStringLength(bdUInt* length)
{
    return bdBool();
}

bdUByte8* bdByteBuffer::getData()
{
    return m_data;
}

bdUInt bdByteBuffer::getSize()
{
    return m_size;
}

bdUInt bdByteBuffer::getMaxWriteSize()
{
    return m_size - getDataSize();
}

bdUInt bdByteBuffer::getMaxReadSize()
{

    return m_size - getReadSize();
}

void bdByteBuffer::setTypeCheck(const bdBool flag)
{
    m_typeChecked = (bdBool)(flag & 1);
}

void bdByteBuffer::allocateBuffer()
{
    if (m_data)
    {
        bdLogWarn("core/bytebuffer", "Buffer already allocated.");
    }
    else
    {
        m_data = bdAllocate<bdUByte8>(m_size);
        m_allocatedData = true;
        m_writePtr = m_data;
        m_readPtr = m_data;
    }
}

bdBool bdByteBuffer::zlibUncompress(bdUByte8** destBuffer, const bdUInt32 destLength, bdUByte8** srcBuffer, const bdUInt32 srcLength)
{
    return bdBool();
}

bdBitBufferDataType bdByteBuffer::inspectDataType()
{
    bdUInt maxReadSize;
    bdBitBufferDataType dataType = BD_BB_NO_TYPE;
    bdUByte8 dataTypeTemp;
    bdUInt offset;

    if (m_typeChecked)
    {
        maxReadSize = getMaxReadSize();
        if (m_readPtr < m_data || maxReadSize > m_size)
        {
            return dataType;
        }
        offset = 0;
        dataTypeTemp = 0;
        if (bdBytePacker::removeBasicType<bdUByte8>(m_readPtr, maxReadSize, 0, &offset, &dataTypeTemp))
        {
            dataType = static_cast<bdBitBufferDataType>(dataTypeTemp);
        }
    }
    return dataType;
}

void bdByteBuffer::resizeToFit(bdUInt32 newDataSize)
{
    bdDeallocate<bdUByte8>(m_data);
    m_data = bdAllocate<bdUByte8>(newDataSize);
    m_readPtr = m_data;
    m_writePtr = m_data;
    m_size = newDataSize;
}

bdBool bdByteBuffer::expand(bdUInt32 increaseSize)
{
    bdUByte8* oldWritePtr;
    bdUByte8* oldReadPtr;
    bdUInt oldSize;
    bdUByte8* oldData;

    if (!m_data)
    {
        bdLogInfo("core/bytebuffer", "Expanding unallocated buffer; allocating with %u Bytes.", increaseSize);
        m_size = increaseSize;
        if (m_data)
        {
            return 1;
        }
        else
        {
            bdAssert(false, "Failed to allocate new buffer.");
            bdLogError("core/bytebuffer", "Could not allocate buffer of size = %u Bytes.", increaseSize);
            return false;
        }
    }

    oldData = m_data;
    oldSize = m_size;
    oldReadPtr = m_readPtr;
    oldWritePtr = m_writePtr;

    if (oldReadPtr < oldData || oldWritePtr < oldData)
    {
        //bdAssert(false, "bdByteBuffer underflow; beginning of data is after read/write pointer positions.");
        return false;
    }

    m_size += increaseSize;
    m_data = NULL;
    m_allocatedData = false;
    // bdReferencable vtable call?
    if (m_data)
    {
        bdMemcpy(m_data, oldData, oldSize);
        m_readPtr = &m_data[oldReadPtr - oldData];
        m_writePtr = &m_data[oldWritePtr - oldData];
        bdDeallocate<bdUByte8>(oldData);
        return true;
    }
    else
    {
        m_data = oldData;
        m_size = oldSize;
        m_readPtr = oldReadPtr;
        m_writePtr = oldWritePtr;
        m_allocatedData = true;
        bdLogError("core/bytebuffer", "Could not allocate new buffer for expansion. Requested size = %u Bytes.", increaseSize + m_size);
        return false;
    }
}

bdBool bdByteBuffer::read(void* data, bdUInt size)
{
    bdUInt maxReadSize;
    bdBool ok = false;
    bdUInt temp;

    maxReadSize = getMaxReadSize();
    if (size > maxReadSize)
    {
        bdLogError("core/bytebuffer", "Could not read data from buffer. Insufficient data available.\n");
    }
    else
    {
        ok = bdBytePacker::removeBuffer(m_readPtr, maxReadSize, 0, &temp, data, size);
        m_readPtr += size;
    }
    return ok;
}

bdBool bdByteBuffer::readArrayStart(bdUByte8 expectedType, bdUInt32* numElements)
{
    bdUInt32 ignoredArrayLength;
    bdUByte8 ignore;
    bdUByte8 type;
    bdBool ok;

    type = 0;
    m_typeCheckedCopy = m_typeChecked;
    m_typeChecked = 0;
    ok = readUByte8(&type);
    if (!ok)
    {
        bdLogWarn("core/bytebuffer", "readArrayStart: No array\n");
        return false;
    }
    if (type - 100 != expectedType)
    {
        bdLogError("core/bytebuffer", "readArrayStart: Expected type %d but read type %d\n", expectedType, type);
        return false;
    }
    m_typeChecked = 0;
    ok = ok && readUByte8(&ignore);
    ok = ok && readUInt32(&ignoredArrayLength);
    ok = ok && readUInt32(numElements);
    return ok;
}

void bdByteBuffer::readArrayEnd()
{
    m_typeChecked = m_typeCheckedCopy;
}

bdBool bdByteBuffer::readUByte8(bdUByte8* b)
{
    if (readDataType(BD_BB_UNSIGNED_CHAR8_TYPE))
    {
        return read<bdUByte8>(b);
    }
    return false;
}

bdBool bdByteBuffer::readUInt32(bdUInt* u)
{
    if (readDataType(BD_BB_UNSIGNED_INTEGER32_TYPE))
    {
        return read<bdUInt>(u);
    }
    return false;
}

bdBool bdByteBuffer::readBool(bdBool* b)
{
    bdBool ok = false;
    bdUByte8 byte = 0;


    if (readDataType(BD_BB_BOOL_TYPE))
    {
        ok = read(&byte, sizeof(bdUByte8));
        if (ok)
        {
            *b = byte ? true : false;
        }
    }
    return ok;
}

bdBool bdByteBuffer::readDataType(const bdBitBufferDataType expectedDataType)
{
    bdNChar8 string2[40];
    bdNChar8 string1[40];
    bdUByte8 dataType8;
    bdBitBufferDataType type;
    bdBool ok = true;

    if (!m_typeChecked)
    {
        return ok;
    }

    dataType8 = 0;
    ok = read(&dataType8, sizeof(dataType8));
    if (!ok)
    {
        return ok;
    }
    type = static_cast<bdBitBufferDataType>(dataType8);
    ok = type == expectedDataType;
    if (!ok)
    {
        bdBitBuffer::typeToString(expectedDataType, string1, sizeof(string1));
        bdBitBuffer::typeToString(type, string2, sizeof(string2));
        bdLogError("core/bytebuffer", "Expected: %s , read: %s ", string1, string2);
    }
    return ok;
}

bdBool bdByteBuffer::readNChar8(bdNChar8* c)
{
    if (readDataType(BD_BB_SIGNED_CHAR8_TYPE))
    {
        return read<bdNChar8>(c);
    }
    return false;
}

bdBool bdByteBuffer::readByte8(bdByte8* b)
{
    if (readDataType(BD_BB_SIGNED_CHAR8_TYPE))
    {
        return read<bdByte8>(b);
    }
    return false;
}

bdBool bdByteBuffer::readInt16(bdInt16* i)
{
    if (readDataType(BD_BB_SIGNED_INTEGER16_TYPE))
    {
        return read<bdInt16>(i);
    }
    return false;
}

bdBool bdByteBuffer::readUInt16(bdUInt16* u)
{
    if (readDataType(BD_BB_UNSIGNED_INTEGER16_TYPE))
    {
        return read<bdUInt16>(u);
    }
    return false;
}

bdBool bdByteBuffer::readInt32(bdInt* i)
{
    if (readDataType(BD_BB_SIGNED_INTEGER32_TYPE))
    {
        return read<bdInt>(i);
    }
    return false;
}

bdBool bdByteBuffer::readInt64(bdInt64* i)
{
    if (readDataType(BD_BB_SIGNED_INTEGER64_TYPE))
    {
        return read<bdInt64>(i);
    }
    return false;
}

bdBool bdByteBuffer::readUInt64(bdUInt64* u)
{
    if (readDataType(BD_BB_UNSIGNED_INTEGER64_TYPE))
    {
        return read<bdUInt64>(u);
    }
    return false;
}

bdBool bdByteBuffer::readFloat32(bdFloat32* f)
{
    if (readDataType(BD_BB_FLOAT32_TYPE))
    {
        return read<bdFloat32>(f);
    }
    return false;
}

bdBool bdByteBuffer::readFloat64(bdFloat64* f)
{
    if (readDataType(BD_BB_FLOAT64_TYPE))
    {
        return read<bdFloat64>(f);
    }
    return false;
}

bdBool bdByteBuffer::readString(bdNChar8* const s, const bdUWord maxLen)
{
    bdBool ok;
    bdUInt i;
    bdNChar8 c;

    ok = readDataType(BD_BB_SIGNED_CHAR8_STRING_TYPE);
    if (ok && s)
    {
        i = 0;
        do
        {
            c = 0;
            ok = read(&c, sizeof(c));
            if (i < maxLen)
            {
                s[i] = c;
            }
            if (ok && c)
            {
                ++i;
            }
        } while (ok && c);
        if (maxLen)
        {
            s[maxLen - 1] = 0;
        }
    }
    return ok;
}

bdBool bdByteBuffer::readBlob(bdUByte8* const blob, bdUInt32* length)
{
    bdBool ok;
    bdUInt32 tempLength;

    ok = readDataType(BD_BB_BLOB_TYPE);

    if (!ok)
    {
        return ok;
    }
    tempLength = 0;
    ok = readUInt32(&tempLength);
    if (ok && blob)
    {
        ok = read(blob, (*length >= tempLength ? tempLength : *length));
        if (tempLength > *length)
        {
            bdLogWarn("core/bytebuffer", "Reading BLOB (%u bytes) buffer too small (%u bytes).", tempLength, *length);
        }
    }
    *length = tempLength;
    return ok;
}

bdBool bdByteBuffer::readAndAllocateBlob(bdUByte8** blob, bdUInt32* length)
{
    bdUByte8* tmpBlob;
    bdUInt32 tmpLength;
    bdBool ok;

    tmpBlob = NULL;
    tmpLength = 0;
    ok = readDataType(BD_BB_BLOB_TYPE);
    if (ok)
    {
        ok = readUInt32(&tmpLength);
        if (ok)
        {
            tmpBlob = bdAllocate<bdUByte8>(tmpLength);
        }
        if (ok && tmpBlob)
        {
            ok = read(tmpBlob, tmpLength);
        }
    }
    if (ok)
    {
        *blob = tmpBlob;
        *length = tmpLength;
    }
    else
    {
        bdDeallocate<bdUByte8>(tmpBlob);
    }
    return ok;
}

// NOT USED IN BO1
bdBool bdByteBuffer::readAndAllocateCompressedMsg(bdUByte8** msg, bdUInt32* length, bdUInt32* compressedLength)
{
    return bdBool();
}

bdBool bdByteBuffer::readNAN()
{
    bdBool ok;

    if (m_typeChecked)
    {
        return readDataType(BD_BB_NAN_TYPE);
    }
    bdLogError("core/bytebuffer", "NaN types cannot be used on not type checked byte buffers");
    return false;
}

bdBitBufferDataType bdByteBuffer::readDataType()
{
    bdBool ok = false;
    bdUByte8 dataTypeTemp = 0;

    bdLogWarn("core/bytebuffer", "bdByteBuffer::readDataType with no parameters is unsafe and deprecated. Use bdByteBuffer::inspectDataType instead.");

    ok = read(&dataTypeTemp, sizeof(dataTypeTemp));
    if (ok)
    {
        return static_cast<bdBitBufferDataType>(dataTypeTemp);
    }
    return BD_BB_NO_TYPE;
}

bdBool bdByteBuffer::write(const void* data, const bdUInt size)
{
    bdUInt32 temp;
    bdUInt32 maxWriteSize;
    bdBool ok = false;

    if (m_data)
    {
        maxWriteSize = getMaxWriteSize();
        if (size > maxWriteSize)
        {
            bdAssert(false, "Could not write data to buffer. Insufficient space.\n");
            bdLogError("core/bytebuffer", "Could not write data to buffer. Insufficient space.");
            return ok;
        }
        ok = bdBytePacker::appendBuffer(m_writePtr, maxWriteSize, 0, &temp, data, size);
        if (ok)
        {
            m_writePtr += temp;
        }
        return ok;
    }
    else
    {
        m_size += size;
        ok = true;
    }
    return ok;
}

bdBool bdByteBuffer::writeArrayStart(bdUByte8 type, bdUInt32 numElements, bdUInt32 elementSize)
{
    bdBool ok;

    m_typeCheckedCopy = m_typeChecked;
    m_typeChecked = false;
    ok = writeUByte8(type + 100);
    m_typeChecked = true;
    ok = ok && writeUInt32(elementSize * numElements);
    m_typeChecked = false;
    return ok && writeUInt32(numElements);
}

void bdByteBuffer::writeArrayEnd()
{
    m_typeChecked = m_typeCheckedCopy;
}

bdBool bdByteBuffer::writeUByte8(bdUByte8 b)
{
    bdBool ok = false;

    if (writeDataType(BD_BB_UNSIGNED_CHAR8_TYPE))
    {
        ok = write<bdUByte8>(&b);
    }
    return ok;
}

bdBool bdByteBuffer::writeBool(bdBool b)
{
    bdBool ok = false;

    if (writeDataType(BD_BB_BOOL_TYPE))
    {
        ok = write<bdBool>(&b);
    }
    return ok;
}

bdBool bdByteBuffer::writeDataType(const bdBitBufferDataType dataType)
{
    bdBool ok = true;
    bdUByte8 dataType8 = dataType;

    if (m_typeChecked)
    {
        ok = write(&dataType8, sizeof(dataType8));
    }
    return ok;
}

bdBool bdByteBuffer::writeNChar8(bdNChar8 c)
{
    bdBool ok = false;

    if (writeDataType(BD_BB_SIGNED_CHAR8_TYPE))
    {
        ok = write<bdNChar8>(&c);
    }
    return ok;
}

bdBool bdByteBuffer::writeByte8(bdByte8 b)
{
    bdBool ok = false;

    if (writeDataType(BD_BB_SIGNED_CHAR8_TYPE))
    {
        ok = write<bdByte8>(&b);
    }
    return ok;
}

bdBool bdByteBuffer::writeInt16(bdInt16 i)
{
    bdBool ok = false;

    if (writeDataType(BD_BB_SIGNED_INTEGER16_TYPE))
    {
        ok = write<bdInt16>(&i);
    }
    return ok;
}

bdBool bdByteBuffer::writeUInt16(bdUInt16 u)
{
    bdBool ok = false;

    if (writeDataType(BD_BB_UNSIGNED_INTEGER16_TYPE))
    {
        ok = write<bdUInt16>(&u);
    }
    return ok;
}

bdBool bdByteBuffer::writeInt32(bdInt i)
{
    bdBool ok = false;

    if (i == 0x7FFFFFFF)
    {
        return writeNAN();
    }
    if (writeDataType(BD_BB_SIGNED_INTEGER32_TYPE))
    {
        ok = write<bdInt>(&i);
    }
    return ok;
}

bdBool bdByteBuffer::writeUInt32(bdUInt u)
{
    bdBool ok = false;

    if (u == -1)
    {
        return writeNAN();
    }
    if (writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE))
    {
        ok = write<bdUInt>(&u);
    }
    return ok;
}

bdBool bdByteBuffer::writeInt64(bdInt64 i)
{
    bdBool ok = false;

    if (i == 0x7FFFFFFFFFFFFFFF)
    {
        return writeNAN();
    }
    if (writeDataType(BD_BB_SIGNED_INTEGER64_TYPE))
    {
        ok = write<bdInt64>(&i);
    }
    return ok;
}

bdBool bdByteBuffer::writeUInt64(bdUInt64 u)
{
    bdBool ok = false;

    if (u == -1LL)
    {
        return writeNAN();
    }
    if (writeDataType(BD_BB_UNSIGNED_INTEGER64_TYPE))
    {
        ok = write<bdUInt64>(&u);
    }
    return ok;
}

bdBool bdByteBuffer::writeFloat32(bdFloat32 f)
{
    bdBool ok = false;

    if (f == 2147483600.0f)
    {
        return writeNAN();
    }
    if (writeDataType(BD_BB_FLOAT32_TYPE))
    {
        ok = write<bdFloat32>(&f);
    }
    return ok;
}

bdBool bdByteBuffer::writeFloat64(bdFloat64 f)
{
    bdBool ok = false;

    if (f == 9.223372036854776e18)
    {
        return writeNAN();
    }
    if (writeDataType(BD_BB_FLOAT64_TYPE))
    {
        ok = write<bdFloat64>(&f);
    }
    return ok;
}

bdBool bdByteBuffer::writeString(const bdNChar8* const s, const bdUWord maxLen)
{
    bdBool ok;
    bdUWord strLength;
    bdBool addNullTerminator;
    bdUInt lengthToWrite;
    bdUByte8 null;

    if (!s)
    {
        return writeNAN();
    }
    ok = writeDataType(BD_BB_SIGNED_CHAR8_STRING_TYPE);
    strLength = bdStrnlen(s, maxLen);
    if (strLength >= maxLen)
    {
        lengthToWrite = maxLen - 1;
        addNullTerminator = true;
        bdLogWarn("core/bytebuffer", "String was not null terminated. Data will be truncated.");
    }
    else
    {
        lengthToWrite = strLength + 1;
        addNullTerminator = false;
    }
    ok = ok && write(s, lengthToWrite);
    if (addNullTerminator)
    {
        null = 0;
        ok = ok && write<bdUByte8>(&null);
    }
    return ok;
}

bdBool bdByteBuffer::writeBlob(const void* const blob, const bdUInt32 length)
{
    bdBool ok;

    if (writeDataType(BD_BB_BLOB_TYPE))
    {
        ok = writeUInt32(length);
    }
    return ok && write(blob, length);
}

bdBool bdByteBuffer::writeNAN()
{
    bdBool ok = false;

    if (m_typeChecked)
    {
        return writeDataType(BD_BB_NAN_TYPE);
    }
    return ok;
}

bdBool bdByteBuffer::writeNoType()
{
    bdBool ok = false;

    if (m_typeChecked)
    {
        return writeDataType(BD_BB_NO_TYPE);
    }
    return ok;
}
