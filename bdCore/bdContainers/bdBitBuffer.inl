// SPDX-License-Identifier: GPL-3.0-or-later

inline bdBitBuffer::bdBitBuffer(const bdUInt capacityBits, const bdBool typeChecked) : bdReferencable(), m_data(((capacityBits & 7) == 0 ? 0 : 1) + (capacityBits >> 3))
{
    bdUByte8 byte;

    m_writePosition = 0;
    m_maxWritePosition = 0;
    m_readPosition = 0;
    m_failedRead = 0;
    m_typeChecked = typeChecked;
    if (m_typeChecked)
    {
        byte = -1;
        writeBits(&byte, 1u);
    }
    else
    {
        byte = 0;
        writeBits(&byte, 1u);
    }
    bdAssert(BD_BB_NUM_HEADER_BITS == m_writePosition, "BD_BB_NUM_HEADER_BITS and written header don't match.");
    resetReadPosition();
}

inline bdBitBuffer::bdBitBuffer(const bdUByte8* bits, const bdUInt numBits, const bdBool dataHasTypeCheckedBit) : bdReferencable(), m_data(0u)
{
    bdUByte8 byte;

    m_writePosition = 0;
    m_maxWritePosition = 0;
    m_readPosition = 0;
    m_failedRead = 0;
    m_typeChecked = 0;
    if (dataHasTypeCheckedBit && numBits)
    {
        m_data.pushBack((bdUByte8*)bits, ((numBits & 7) != 0) + (numBits >> 3));
        m_writePosition = numBits;
        m_maxWritePosition = numBits;
        readBits(&m_typeChecked, 1u);
    }
    else
    {
        byte = 0;
        writeBits(&byte, 1u);
        writeBits(bits, numBits);
        m_readPosition = 1;
    }
}

inline bdBitBuffer::~bdBitBuffer()
{
}

inline void* bdBitBuffer::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

inline void bdBitBuffer::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

inline void bdBitBuffer::writeBits(const void* bits, const bdUInt numBits)
{
    bdUByte8 nextSrcByte;
    bdUByte8 currentSrcByte;
    bdUInt currentSrcByteIndex;
    bdUByte8 maskedDest;
    bdUInt nextDestByteIndex;
    bdUByte8 mask;
    bdInt upShift;
    bdUInt bitsToWrite;
    bdUInt lastByteIndex;

    lastByteIndex = (numBits + m_writePosition - 1) / 8;
    if (!m_data.rangeCheck(lastByteIndex))
    {
        bdUByte8 value = 0;
        m_data.setGrow(lastByteIndex, value);
    }
    bitsToWrite = numBits;
    while (bitsToWrite)
    {
        upShift = m_writePosition & 7;
        if (bitsToWrite >= 8 - upShift)
        {
            bitsToWrite = 8 - upShift;
        }
        mask = (255 >> (8 - upShift)) | (255 << (upShift + bitsToWrite));
        nextDestByteIndex = m_writePosition / 8;
        maskedDest = mask & m_data[nextDestByteIndex];
        currentSrcByteIndex = (numBits - bitsToWrite) / 8;
        nextSrcByte = 0;
        if ((numBits - 1) / 8 > currentSrcByteIndex)
        {
            nextSrcByte = ((bdUByte8*)bits)[currentSrcByteIndex + 1];
        }
        currentSrcByte = ((bdUByte8*)bits)[((numBits - bitsToWrite) / 8)];
        m_data[nextDestByteIndex] = ~mask & ((bdUByte8)((nextSrcByte << (8 - ((numBits - bitsToWrite) & 7))) | ((bdInt)currentSrcByte >> ((numBits - bitsToWrite) & 7))) << upShift) | maskedDest;
        m_writePosition += bitsToWrite;
        bitsToWrite -= bitsToWrite;
        if (m_maxWritePosition <= m_writePosition)
        {
            m_writePosition = m_writePosition;
        }
        else
        {
            m_writePosition = m_maxWritePosition;
        }
        m_maxWritePosition = m_writePosition;
    }
}

inline bdBool bdBitBuffer::writeBool(const bdBool b)
{
    writeDataType(BD_BB_BOOL_TYPE);
    if (b)
    {
        bdUByte8 byte = -1;
        writeBits(&byte, 1u);
    }
    else
    {
        bdUByte8 byte = 0;
        writeBits(&byte, 1u);
    }
    return getData();
}

inline void bdBitBuffer::writeUInt32(const bdUInt32 u)
{
    bdUInt nu;

    writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    bdBitOperations::endianSwap<bdUInt>(u, nu);
    writeBits(&nu, 32u);
}

inline void bdBitBuffer::writeInt32(const bdInt32 i)
{
    bdInt ni;

    writeDataType(BD_BB_SIGNED_INTEGER32_TYPE);
    bdBitOperations::endianSwap<bdInt32>(i, ni);
    writeBits(&ni, 32u);
}

inline void bdBitBuffer::writeFloat32(const bdFloat32 f)
{
    bdFloat32 nf;

    writeDataType(BD_BB_FLOAT32_TYPE);
    bdBitOperations::endianSwap<bdFloat32>(f, nf);
    writeBits(&nf, 32);
}

inline void bdBitBuffer::writeRangedUInt32(const bdUInt u, const bdUInt begin, const bdUInt end, const bdBool typeChecked)
{
    bdUInt rangeBits;
    bdUInt newEnd;
    bdUInt norm, newNorm;

    bdAssert(end >= begin, "bdBitBuffer::writeRangedUInt, end of range is less than the begining");
    if (typeChecked)
    {
        writeDataType(BD_BB_RANGED_UNSIGNED_INTEGER32_TYPE);
        if (m_typeChecked)
        {
            writeUInt32(begin);
            writeUInt32(end);
        }
    }
    rangeBits = 0;
    if (end != begin)
    {
        rangeBits = bdBitOperations::highBitNumber(end - begin) + 1;
    }
    if (u <= end)
    {
        if (u >= begin)
        {
            newEnd = u;
        }
        else
        {
            newEnd = begin;
        }
    }
    else
    {
        newEnd = end;
    }
    norm = newEnd - begin;
    bdBitOperations::endianSwap<bdUInt>(norm, newNorm);
    writeBits(&newNorm, rangeBits);
}

inline void bdBitBuffer::writeDataType(const bdBitBufferDataType dataType)
{
    if (m_typeChecked)
    {
        writeRangedUInt32(dataType, 0, 31, false);
    }
}

inline void bdBitBuffer::writeRangedInt32(const bdInt i, const bdInt begin, const bdInt end)
{
    bdInt newEnd;
    bdUInt rangeBits;
    bdInt norm, newNorm;

    writeDataType(BD_BB_RANGED_SIGNED_INTEGER32_TYPE);
    if (m_typeChecked)
    {
        writeInt32(begin);
        writeInt32(end);
    }
    rangeBits = 0;
    if (end != begin)
    {
        rangeBits = bdBitOperations::highBitNumber(end - begin) + 1;
    }
    if (i <= end)
    {
        if (i >= begin)
        {
            newEnd = i;
        }
        else
        {
            newEnd = begin;
        }
    }
    else
    {
        newEnd = end;
    }
    norm = newEnd - begin;
    bdBitOperations::endianSwap<bdInt>(norm, newNorm);
    writeBits(&newNorm, rangeBits);
}

inline void bdBitBuffer::writeRangedFloat32(const bdFloat32 f, const bdFloat32 begin, const bdFloat32 end, const bdFloat32 precision)
{
    bdFloat32 newPrecision;
    bdFloat32 range;
    bdFloat32 newEnd;
    bdFloat32 newVal;
    bdUInt32 rangeBits;
    bdUInt i;
    bdUInt ni;

    bdAssert(end >= begin, "bdBitBuffer::writeRangedFloat32, end of range is less then the begining.");
    bdAssert(precision > 0.0, "bdBitBuffer::writeRangedFloat32, precision must be positive.");
    writeDataType(BD_BB_RANGED_FLOAT32_TYPE);
    if (m_typeChecked)
    {
        writeFloat32(begin);
        writeFloat32(end);
        writeFloat32(precision);
    }

    if (precision <= 0.0f)
    {
        newPrecision = -precision;
    }
    else
    {
        newPrecision = precision;
    }
    range = (end - begin) / newPrecision;
    if (range > 4294967300.0f)
    {
        bdLogWarn("bdCore/bitBuffer", "The numerical space defined by range/precision combination is too large. No compression performed.");
        writeFloat32(f);
        return;
    }
    rangeBits = 0;
    if (range)
    {
        rangeBits = bdBitOperations::highBitNumber(range) + 1;
    }
    if (f <= end)
    {
        if (begin <= f)
        {
            newEnd = f;
        }
        else
        {
            newEnd = begin;
        }
    }
    else
    {
        newEnd = end;
    }
    newVal = ((newEnd - begin) / newPrecision) + 0.5;
    if (newVal > range)
    {
        newVal = (end - begin) / newPrecision;
    }
    i = newVal;
    bdBitOperations::endianSwap<bdUInt>(i, ni);
    writeBits(&ni, rangeBits);
}

inline bdBool bdBitBuffer::readBits(void* bits, bdUInt numBits)
{
    bdUInt downShift;
    bdUInt nextByteIndex;
    bdUByte8 byte0;
    bdUByte8* dest;
    bdUByte8 value;

    if (!numBits)
    {
        return 1;
    }
    if (numBits + this->m_readPosition > this->m_maxWritePosition)
    {
        this->m_failedRead = 1;
        return false;
    }
    nextByteIndex = this->m_readPosition >> 3;
    while (numBits)
    {
        if (!m_data.rangeCheck(nextByteIndex))
        {
            this->m_failedRead = 1;
            return false;
        }
        if (numBits >= 8)
        {
            numBits = 8;
        }
        byte0 = m_data[++nextByteIndex];
        downShift = m_readPosition & 7;
        if (numBits + downShift <= 8)
        {
            dest = reinterpret_cast<bdUByte8*>(bits);
            bits = (char*)bits + 1;
            *dest = (255 >> (8 - numBits)) & (byte0 >> downShift);
        }
        else
        {
            if (!m_data.rangeCheck(nextByteIndex))
            {
                m_failedRead = 1;
                return false;
            }
            value = (255 >> (8 - numBits)) & ((m_data[nextByteIndex] << (8 - downShift)) | (byte0 >> downShift));
            dest = reinterpret_cast<bdUByte8*>(bits);
            bits = (char*)bits + 1;
            *dest = value;
        }
        m_readPosition += numBits;
        numBits -= numBits;
    }
    return true;
}

inline bdBool bdBitBuffer::readUInt32(bdUInt& u)
{
    bdBool ok = false;
    bdUInt nu;

    if (readDataType(BD_BB_UNSIGNED_INTEGER32_TYPE))
    {
        ok = readBits(&nu, 32);
    }
    if (ok)
    {
        bdBitOperations::endianSwap<bdUInt>(nu, u);
    }
    return ok;
}

inline bdBool bdBitBuffer::readInt32(bdInt32& i)
{
    bdBool ok = false;
    bdInt32 ni;

    if (readDataType(BD_BB_SIGNED_INTEGER32_TYPE))
    {
        ok = readBits(&ni, 32);
    }
    if (ok)
    {
        bdBitOperations::endianSwap<bdInt32>(ni, i);
    }
    return ok;
}

inline bdBool bdBitBuffer::readFloat32(bdFloat32& f)
{
    bdBool ok = false;
    bdFloat32 nf;

    if (readDataType(BD_BB_FLOAT32_TYPE))
    {
        ok = readBits(&nf, 32);
    }
    if (ok)
    {
        bdBitOperations::endianSwap<bdFloat32>(nf, f);
    }
    return ok;
}

inline bdBool bdBitBuffer::readRangedUInt32(bdUInt& u, const bdUInt begin, const bdUInt end, const bdBool typeChecked)
{
    bdUInt32 bufEnd;
    bdUInt32 bufBegin;
    bdUInt rangeBits;
    bdUInt rangeSize;
    bdUInt newBegin;
    bdUInt nu;
    bdBool ok;

    bdAssert(end >= begin, "bdBitBuffer::writeRangedUInt, end of range is less than the begining");
    ok = true;
    if (typeChecked)
    {
        ok = readDataType(BD_BB_RANGED_UNSIGNED_INTEGER32_TYPE);
        if (m_typeChecked)
        {
            bufBegin = 0;
            bufEnd = 0;
            if (ok)
            {
                ok = readUInt32(bufBegin);
            }
            if (ok)
            {
                ok = readUInt32(bufEnd);
            }
            if (ok && (begin != bufBegin || end != bufEnd))
            {
                bdLogError("bdCore/bitBuffer", "Range error. Expected: (%u,%u), read: (%u,%u)", begin, end, bufBegin, bufEnd);
            }
        }
    }
    if (ok)
    {
        rangeSize = end - begin;
        rangeBits = 0;
        if (end != begin)
        {
            rangeBits = bdBitOperations::highBitNumber(rangeSize) + 1;
        }
        nu = 0;
        if (ok)
        {
            ok = readBits(&nu, rangeBits);
        }
        if (ok)
        {
            bdBitOperations::endianSwap<bdUInt>(nu, u);
            u += begin;
            bdAssert(u >= begin && u <= end, "bdBitBuffer::readRangedUInt32, read error u is out of range.");
            if (u <= end)
            {
                if (u >= begin)
                {
                    newBegin = u;
                }
                else
                {
                    newBegin = begin;
                }
            }
            else
            {
                newBegin = end;
            }
            u = newBegin;
        }
    }
    return ok;
}

inline bdBool bdBitBuffer::readDataType(const bdBitBufferDataType expectedDataType)
{
    bdBool ok = true;
    bdUInt32 dataType32;
    bdNChar8 string2[40];
    bdNChar8 string1[40];

    if (!this->m_typeChecked)
    {
        return ok;
    }
    dataType32 = 0;
    ok = readRangedUInt32(dataType32, 0, 31u, false);
    if (ok)
    {
        ok = dataType32 == expectedDataType;
        bdBitBuffer::typeToString(expectedDataType, string1, sizeof(string1));
        bdBitBuffer::typeToString((const bdBitBufferDataType)dataType32, string2, sizeof(string2));
    }
    return ok;
}

inline bdBool bdBitBuffer::readRangedInt32(bdInt& i, const bdInt begin, const bdInt end)
{
    bdInt32 bufEnd;
    bdInt32 bufBegin;
    bdInt rangeBits;
    bdInt rangeSize;
    bdInt newBegin;
    bdInt ni;
    bdBool ok;

    //bdAssertMsg(end >= begin, "bdBitBuffer::writeRangedUInt, end of range is less than the begining");
    ok = true;
    ok = readDataType(BD_BB_RANGED_UNSIGNED_INTEGER32_TYPE);
    if (this->m_typeChecked)
    {
        bufBegin = 0;
        bufEnd = 0;
        if (ok)
        {
            ok = readInt32(bufBegin);
        }
        if (ok)
        {
            ok = readInt32(bufEnd);
        }
        if (ok && (begin != bufBegin || end != bufEnd))
        {
            bdLogMessage(
                BD_LOG_ERROR,
                "err/",
                "bdCore/bitBuffer",
                __FILE__,
                __FUNCTION__,
                __LINE__,
                "Range error. Expected: (%u,%u), read: (%u,%u)");
        }
    }
    if (ok)
    {
        rangeSize = end - begin;
        rangeBits = 0;
        if (end != begin)
        {
            rangeBits = bdBitOperations::highBitNumber(rangeSize) + 1;
        }
        ni = 0;
        if (ok)
        {
            ok = readBits(&ni, rangeBits);
        }
        if (ok)
        {
            bdBitOperations::endianSwap<bdInt>(ni, i);
            i += begin;
            bdAssert(i >= begin && i <= end, "bdBitBuffer::readRangedUInt32, read error u is out of range.");
            if (i <= end)
            {
                if (i >= begin)
                {
                    newBegin = i;
                }
                else
                {
                    newBegin = begin;
                }
            }
            else
            {
                newBegin = end;
            }
            i = newBegin;
        }
    }
    return ok;
}

inline bdBool bdBitBuffer::readRangedFloat32(bdFloat32& f, const bdFloat32 begin, const bdFloat32 end, const bdFloat32 precision)
{
    bdUInt rangeBits;
    float range;
    bdBool ok;
    bdUInt i;
    bdUInt ni;
    bdFloat32 newPrecision;
    bdFloat32 bufPrecision;
    bdFloat32 bufEnd;
    bdFloat32 bufBegin;
    bdFloat32 newBegin;

    bdAssert(end >= begin, "bdBitBuffer::writeRangedFloat32, end of range is less then the begining.");
    bdAssert(precision > 0, "bdBitBuffer::writeRangedFloat32, precision must be positive.");

    ok = readDataType(BD_BB_RANGED_FLOAT32_TYPE);
    if (m_typeChecked)
    {
        bufBegin = 0.0;
        bufEnd = 0.0;
        bufPrecision = 0.0;
        if (ok)
        {
            ok = readFloat32(bufBegin);
        }
        if (ok)
        {
            ok = readFloat32(bufEnd);
        }
        if (ok)
        {
            ok = readFloat32(bufPrecision);
        }
        if (ok && (begin != bufBegin || end != bufEnd || precision != bufPrecision))
        {
            bdLogError("bdCore/bitBuffer", "Range error. Expected: (%f,%f,%f), read: (%f,%f,%f)", begin, end, precision, bufBegin, bufEnd, bufPrecision);
        }
    }
    if (ok)
    {
        if (precision <= 0.0)
        {
            newPrecision = -precision;
        }
        else
        {
            newPrecision = precision;
        }
        range = (end - begin) / newPrecision;
        
        if (range <= 4294967300.0)
        {
            rangeBits = 0;
            if (range)
            {
                rangeBits = bdBitOperations::highBitNumber(range) + 1;
            }
            ni = 0;
            ok = readBits(&ni, rangeBits);
            if (ok)
            {
                bdBitOperations::endianSwap<bdUInt>(ni, i);
                f = begin + ((bdFloat32)i * newPrecision);
            }
        }
        else
        {
            bdLogWarn("bdCore/bitBuffer", "The numerical space defined by range/precision combination is too large. No compression performed.");
            ok = readFloat32(f);
        }

        if (ok)
        {
            bdAssert(f >= begin && f <= end, "bdBitBuffer::readRangedFloat32, read error f is out of range.");
            if (f <= end)
            {
                if (begin <= f)
                {
                    newBegin = f;
                }
                else
                {
                    newBegin = begin;
                }
            }
            else
            {
                newBegin = end;
            }
            f = newBegin;
        }
    }
    return ok;
}

inline void bdBitBuffer::setTypeCheck(const bdBool flag)
{
    m_typeChecked = flag;
}

inline bdBool bdBitBuffer::getTypeCheck()
{
    return m_typeChecked;
}

inline bdUByte8* bdBitBuffer::getData()
{
    return m_data.begin();
}

inline void bdBitBuffer::typeToString(const bdBitBufferDataType type, bdNChar8* const strBuffer, const bdUWord strLength)
{
    bdBitBufferDataType newType;
    const char* dataTypeDescs[] = { "NoType", "Bool", "Char8", "UChar8", "WChar16", "Int16", "UInt16", 
        "Int32", "UInt32", "RangedInt32", "RangeUInt32", "Float32", "Float64", "RangeFloat32", "String", "String",
        "MultiByteString", "Blob", "NaN", "FullType", "Unknown Type"};

    if (type <= (BD_BB_NAN_TYPE | BD_BB_SIGNED_CHAR8_TYPE))
        newType = type < BD_BB_NO_TYPE ? BD_BB_NO_TYPE : type;
    else
        newType = static_cast<bdBitBufferDataType>(BD_BB_NAN_TYPE | BD_BB_SIGNED_CHAR8_TYPE);
    bdStrlcpy(strBuffer, dataTypeDescs[newType], strLength);
}

inline bdBitBufferDataType bdBitBuffer::readDataType()
{
    bdBool ok;
    bdUInt32 dataType32 = 0;

    ok = readRangedUInt32(dataType32, 0, 31u, 0);
    
    return static_cast<bdBitBufferDataType>(dataType32);

}

inline bdBool bdBitBuffer::append(bdBitBuffer& other)
{
    bdUInt numBits;
    void* tempData;
    bdUInt oldReadPos;

    bdBool ok = m_typeChecked == other.m_typeChecked;
    if (ok)
    {
        oldReadPos = other.getReadPosition();
        other.resetReadPosition();
        tempData = bdMemory::allocate(other.getDataSize());
        numBits = other.getNumBitsWritten();
        if (tempData)
        {
            ok = other.readBits(tempData, numBits);
        }
        if (ok)
        {
            writeBits(tempData, numBits);
        }
        bdMemory::deallocate(tempData);
        other.setReadPosition(oldReadPos);
    }
    else
    {
        bdLogError("bdCore/bitBuffer", "Attempt made to append a bdBitBuffer that %s to a bdBitBuffer that %s.",
            (!m_typeChecked ? "is type checked" : "is not type checked"), (!other.m_typeChecked ? "is type checked" : "is not type checked"));
    }
    return ok;
}

inline void bdBitBuffer::resetReadPosition()
{
    m_readPosition = 1;
}

inline bdUInt bdBitBuffer::getReadPosition()
{
    return m_readPosition;
}

inline bdUInt bdBitBuffer::getDataSize()
{
    return m_data.getSize();
}

inline bdUInt bdBitBuffer::getNumBitsWritten()
{
    bdUInt numBitsWritten = 0;

    if (m_writePosition == 0)
        return numBitsWritten;
    numBitsWritten = m_maxWritePosition - 1;
    return numBitsWritten;
}

inline void bdBitBuffer::setReadPosition(const bdUInt bitPosition)
{
    bdUInt newBitPosition = bitPosition;

    if (newBitPosition < 2)
        newBitPosition = 1;
    m_readPosition = newBitPosition;
}
