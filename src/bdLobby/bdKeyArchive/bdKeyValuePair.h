// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdKeyValuePair : public bdTaskResult
{
public:
    enum bdArchiveUpdateType
    {
        BD_REPLACE = 0,
        BD_ADD = 1,
        BD_MAX = 2,
        BD_MIN = 3,
        BD_AND = 4,
        BD_OR = 5,
        BD_XOR = 6
    };
    bdUByte8 m_updateType;
    bdUInt16 m_index;
    bdInt64 m_value;
};