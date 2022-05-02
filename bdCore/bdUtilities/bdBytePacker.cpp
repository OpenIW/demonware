// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdBool bdBytePacker::appendBuffer(bdUByte8* dest, bdUInt destSize, bdUInt offset, bdUInt* newOffset, const void* src, bdUInt writeSize)
{
	*newOffset = writeSize + offset;

	if (dest && src)
	{
		if (*newOffset > destSize)
		{
			bdLogMessage(BD_LOG_WARNING, "warn/", "byte packer", __FILE__, "bdBytePacker::appendBuffer", __LINE__, "Not enough room left to write %u bytes.", writeSize);
		}

		if (offset <= destSize && *newOffset <= destSize)
		{
			memmove(&dest[offset], src, writeSize);
			return true;
		}
	}
	return false;
}

bdBool bdBytePacker::appendEncodedUInt16(bdUByte8* buffer, bdUInt bufferSize, bdUInt offset, bdUInt* newOffset, bdUInt16 value)
{
	char src;

	if (value <= 127)
	{
		return appendBuffer(buffer, bufferSize, offset, newOffset, (char*)&value, 1);
	}
	else
	{
		src = value | 0x8000;
		if (appendBuffer(buffer, bufferSize, offset, newOffset, (char*)&src + 3, 1))
		{
			if (appendBuffer(buffer, bufferSize, *newOffset, newOffset, (char*)&src, 1u))
			{
				return true;
			}
		}
		return false;
	}
}

bdBool bdBytePacker::removeBuffer(const bdUByte8* src, bdUInt srcSize, bdUInt offset, bdUInt* newOffset, void* const dest, bdUInt readSize)
{
	*newOffset = readSize + offset;

	if (dest && src)
	{
		if (*newOffset > srcSize)
		{
			bdLogMessage(BD_LOG_WARNING, "warn/", "byte packer", __FILE__, __FUNCTION__, __LINE__, "Not enough data left to read %u bytes.", readSize);
		}

		if ((*newOffset <= srcSize) && offset <= srcSize)
		{
			memmove(dest, &src[offset], readSize);
			return true;
		}
	}
	return false;
}

bdBool bdBytePacker::removeEncodedUInt16(const bdUByte8* buffer, bdUInt bufferSize, bdUInt offset, bdUInt* newOffset, bdUInt16* value)
{
	unsigned char firstByte = 0;

	if (!removeBuffer(buffer, bufferSize, offset, newOffset, &firstByte, 1))
	{
		*value = 0;
		return false;
	}

	if (firstByte < 0x80)
	{
		*value = firstByte;
		return true;
	}

	firstByte &= ~0x80;
	if (removeBuffer(buffer, bufferSize, *newOffset, newOffset, &firstByte, 1))
	{
		*value = firstByte + (firstByte << 8);
		return true;
	}
	else
	{
		*value = (firstByte << 8);
		return false;
	}
}

bdBool bdBytePacker::skipBytes(const bdUByte8* buffer, bdUInt bufferSize, bdUInt offset, bdUInt* newOffset, bdUInt bytes)
{
	*newOffset = bytes + offset;

	if (buffer)
	{
		return bufferSize >= *newOffset;
	}
	return false;
}

bdBool bdBytePacker::rewindBytes(const bdUByte8* buffer, bdUInt bufferSize, bdUInt offset, bdUInt* newOffset, bdUInt bytes)
{
	*newOffset = offset - bytes;

	if (buffer)
	{
		return offset >= bytes;
	}
	return false;
}

template<typename varType>
inline bdBool bdBytePacker::removeBasicType(const void* buffer, const bdUInt bufferSize, bdUInt offset, bdUInt* newOffset, varType* var)
{
	bdBool read;
	varType nvar;

	read = removeBuffer(buffer, bufferSize, offset, newOffset, &nvar, sizeof(varType));
	if (read)
	{
		buffer = &nvar;
		bdBitOperations::endianSwap<varType>(&nvar, var);
	}
	return read;
}

template<typename varType>
bdBool bdBytePacker::appendBasicType(void* buffer, const bdUInt bufferSize, bdUInt offset, bdUInt* newOffset, const varType* var)
{
	varType nvar;

	bdBitOperations::endianSwap<varType>(var, &nvar);
	return appendBuffer(buffer, bufferSize, offset, newOffset, &nvar, sizeof(varType));
}
