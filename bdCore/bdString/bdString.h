/*
 * DemonWare
 * Copyright (c) 2020-2022 OpenIW
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

struct bdStringData
{
    unsigned int m_referenceCount;
    unsigned int m_length;
    unsigned int m_capacity;
};

static bdNChar8* g_emptyString;

struct bdEmptyStringStatic
{
    bdStringData m_stringData;
    bdNChar8 m_stringByte;

    bdEmptyStringStatic()
    {
        m_stringData.m_referenceCount = 1;
        m_stringData.m_capacity = 0;
        m_stringData.m_length = 0;
        m_stringByte = *g_emptyString;
    }
};

static bdEmptyStringStatic* g_emptyStringData;

static bdStringData* getEmptyStringData()
{
    *g_emptyString = 0;
    return &g_emptyStringData->m_stringData;
}

class bdString
{
public:
    char *m_string;

    bdString(const bdString *s)
    {
        bdStringData *StringData;

        this->m_string = s->m_string;
        addReference(getStringData());
    }
    bdString(const char *s)
    {
        bdUWord length;

        length = bdStrlen(s);

        if (length)
        {
            allocateBuffer(length);
            bdMemcpy(m_string, s, length + 1);
        }
        else
        {
            initialize();
        }
    }
    ~bdString()
    {
        removeReference(getStringData());
    }
    bdString *operator=(char **s)
    {
        if (m_string == *s)
        {
            return this;
        }
        removeReference(getStringData());
        m_string = *s;
        addReference(getStringData());
    }
    bdStringData* getStringData()
    {
        return reinterpret_cast<bdStringData*>((m_string - sizeof(bdStringData)));
    }
    void initialize()
    {
        m_string = reinterpret_cast<bdNChar8*>((getEmptyStringData() + sizeof(bdStringData)));
        addReference(getStringData());
    }
    void addReference(bdStringData* stringData)
    {
        ++stringData->m_referenceCount;
    }
    void removeReference(bdStringData* stringData)
    {
        if (!--stringData->m_referenceCount)
            freeBuffer(stringData);
    }
    void allocateBuffer(unsigned int length)
    {
        bdStringData* stringData;
        bdUWord numChunks;

        numChunks = (length + 1) >> 6;
        if ((length + 1) % 0x40)
        {
            ++numChunks;
        }
        stringData = reinterpret_cast<bdStringData*>(bdAllocate<char>((numChunks << 6) + sizeof(bdStringData)));
        stringData->m_referenceCount = 1;
        stringData->m_capacity = numChunks << 6;
        stringData->m_length = length;
    }
    void freeBuffer(bdStringData* stringData)
    {
        bdMemory::deallocate(stringData);
    }
    char* getBuffer()
    {
        return m_string;
    }
};
