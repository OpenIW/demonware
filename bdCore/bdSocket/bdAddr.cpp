// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdAddr::bdAddr()
{
	m_address = bdInetAddr();
	m_port = 0;
}

bdAddr::bdAddr(const bdAddr* other)
{
	m_address = bdInetAddr(other->m_address);
	m_port = other->m_port;
}

bdAddr::bdAddr(const bdInetAddr* address, const bdPort port)
{
	m_address = bdInetAddr(address);
	m_port = port;
}

bdAddr::bdAddr(const bdNChar8* socketAddress)
{
	m_address = bdInetAddr();
	set(socketAddress);
}

void bdAddr::set(const bdNChar8* socketAddress)
{
	const bdNChar8* sep;
	bdUInt pos;
	bdUInt strAddrSize;
	bdNChar8 strAddr[16];

	sep = bdStrchr(socketAddress, 58);
	if (sep)
	{
		pos = sep - socketAddress + 1;
		strAddrSize = 16;
		if (pos <= 16)
		{
			bdMemcpy(strAddr, socketAddress, pos);
		}
		else
		{
			bdMemcpy(strAddr, socketAddress, sizeof(strAddr));
		}

		if (pos <= 16)
		{
			pos -= 1;
		}
		else
		{
			pos = 15;
		}

		strAddr[pos] = 0;
		m_address.set(bdInAddr(strAddr).inUn.m_iaddr);
		m_port = strtoul(sep + 1, 0, 10);
	}
	else
	{
		set(&bdInetAddr(socketAddress), 0);
	}
}

void bdAddr::set(const bdInetAddr* address, const bdPort port)
{
	m_address.set(address);
	m_port = port;
}

void bdAddr::setPort(const bdPort port)
{
	m_port = port;
}

bdUInt bdAddr::getHash()
{
	bdUInt hash = 0;
	bdNChar8 val[1288];
	bdUInt len;

	serialize(reinterpret_cast<bdUByte8*>(val), sizeof(val), 0, &len);
	for (bdUInt i = 0; i < len; ++i)
	{
		hash = val[i] ^ (0x1000193 * hash);
	}
	return hash;
}

bdInetAddr* bdAddr::getAddress()
{
	return &m_address;
}

bdPort bdAddr::getPort()
{
	return m_port;
}

bdUInt bdAddr::getSerializedSize()
{
	if (!serializedSize)
	{
		serialize(NULL, 65535, 0, &serializedSize);
	}
	return serializedSize;
}

const bdUWord bdAddr::toString(bdNChar8* const str, const bdUWord size) const
{
	bdUWord strLength;

	strLength = m_address.toString(str, size);
	return bdSnprintf(&str[strLength], (strLength <= size ? size - strLength : 0), ":%u") + strLength;
}

bdBool bdAddr::serialize(bdUByte8* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset)
{
	bdBool ok;

	*newOffset = offset;
	if (m_address.serialize(data, size, *newOffset, newOffset))
	{
		ok = bdBytePacker::appendBasicType<bdPort>(data, size, *newOffset, newOffset, &m_port);
	}
	if (!ok)
	{
		*newOffset = offset;
	}
	return ok;
}

bdBool bdAddr::deserialize(const bdUByte8* data, const bdUInt size, const bdUInt offset, bdUInt* newOffset)
{
	bdBool ok;

	*newOffset = offset;
	if (m_address.deserialize(data, size, *newOffset, newOffset))
	{
		ok = bdBytePacker::removeBasicType<bdPort>(data, size, *newOffset, newOffset, &m_port);
	}
	if (!ok)
	{
		*newOffset = offset;
	}
	return ok;
}
