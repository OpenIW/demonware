// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdByteBuffer : public bdReferencable
{
protected:
    bdUInt m_size;
    bdUByte8* m_data;
    bdUByte8* m_readPtr;
    bdUByte8* m_writePtr;
    bdBool m_typeChecked;
    bdBool m_typeCheckedCopy;
    bdBool m_allocatedData;
public:
    void* operator new(bdUWord nbytes)
    {
        return bdMemory::allocate(nbytes);
    }
    void operator delete(void* p)
    {
        bdMemory::deallocate(p);
    }
    bdByteBuffer();
    bdByteBuffer(void* bytes, const bdUInt size, bdBool isTypeChecked);
    bdByteBuffer(const bdUInt size, bdBool isTypeChecked);
    virtual ~bdByteBuffer();

    const bdUInt getDataSize() const;
    const bdUInt getReadSize() const;
    bdBool getStringLength(bdUInt& length);
    bdUByte8* getData() const;
    const bdUInt getSize() const;
    const bdUInt getMaxWriteSize() const;
    const bdUInt getMaxReadSize() const;

    void setTypeCheck(const bdBool flag);
    void allocateBuffer();
    bdBitBufferDataType inspectDataType();
    void resizeToFit(bdUInt32 newDataSize);
    bdBool expand(bdUInt32 increaseSize);

    bdBool read(void* data, bdUInt size);
    bdBool readArrayStart(bdUByte8 expectedType, bdUInt32& numElements);
    void readArrayEnd();
    bdBool readUByte8(bdUByte8& b);
    bdBool readUInt32(bdUInt& u);
    bdBool readBool(bdBool& b);
    bdBool readDataType(const bdBitBufferDataType expectedDataType);
    bdBool readNChar8(bdNChar8& c);
    bdBool readByte8(bdByte8& b);
    bdBool readInt16(bdInt16& i);
    bdBool readUInt16(bdUInt16& u);
    bdBool readInt32(bdInt& i);
    bdBool readInt64(bdInt64& i);
    bdBool readUInt64(bdUInt64& u);
    bdBool readFloat32(bdFloat32& f);
    bdBool readFloat64(bdFloat64& f);
    bdBool readString(bdNChar8* const s, const bdUWord maxLen);
    bdBool readBlob(bdUByte8* const blob, bdUInt32& length);
    bdBool readAndAllocateBlob(bdUByte8*& blob, bdUInt32& length);
    bdBool readAndAllocateCompressedMsg(bdUByte8*& msg, bdUInt32& length, bdUInt32& compressedLength);
    bdBool readNAN();
    bdBitBufferDataType readDataType();
    template <typename dataType>
    bdBool read(dataType& var)
    {
        bdBool result;
        dataType temp;

        result = read(reinterpret_cast<void*>(&temp), sizeof(dataType));
        if (result)
        {
            bdBitOperations::endianSwap<dataType>(temp, var);
        }
        return result;
    }

    bdBool write(const void* data, const bdUInt size);
    bdBool writeArrayStart(bdUByte8 type, bdUInt32 numElements, bdUInt32 elementSize);
    void writeArrayEnd();
    bdBool writeUByte8(bdUByte8 b);
    bdBool writeBool(bdBool b);
    bdBool writeDataType(const bdBitBufferDataType dataType);
    bdBool writeNChar8(bdNChar8 c);
    bdBool writeByte8(bdByte8 b);
    bdBool writeInt16(bdInt16 i);
    bdBool writeUInt16(bdUInt16 u);
    bdBool writeInt32(bdInt i);
    bdBool writeUInt32(bdUInt u);
    bdBool writeInt64(bdInt64 i);
    bdBool writeUInt64(bdUInt64 u);
    bdBool writeFloat32(bdFloat32 f);
    bdBool writeFloat64(bdFloat64 f);
    bdBool writeString(const bdNChar8* const s, const bdUWord maxLen);
    bdBool writeBlob(const void* const blob, const bdUInt32 length);
    bdBool writeNAN();
    bdBool writeNoType();
    template <typename dataType>
    bdBool write(const dataType& var)
    {
        dataType evar;

        bdBitOperations::endianSwap<dataType>(var, evar);
        return write(reinterpret_cast<void*>(&evar), sizeof(dataType));
    }
};

typedef bdReference<bdByteBuffer> bdByteBufferRef;
