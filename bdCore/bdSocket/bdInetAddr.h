// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdInetAddr
{
protected:
	bdInAddr m_addr;
public:
	bool operator == (bdInetAddr* comp);
	static bdInetAddr Broadcast();
	static bdInetAddr All();
	static bdInetAddr Any();
	bdInetAddr();
	~bdInetAddr();
	bdInetAddr(const bdInetAddr* other);
	bdInetAddr(const char* address);
	bdInetAddr(unsigned int address);
	bdInetAddr(bdInAddr address);
	void set(const bdInetAddr* other);
	void set(const char* address);
	void set(unsigned int address);
	const bdBool isValid() const;
	const bdUInt toString(char* str, unsigned int size) const;
	bdInAddr getInAddr();
	bool serialize(bdUByte8* data, unsigned int size, unsigned int offset, unsigned int* newOffset);
	bool deserialize(const bdUByte8* data, unsigned int size, unsigned int offset, unsigned int* newOffset);
};