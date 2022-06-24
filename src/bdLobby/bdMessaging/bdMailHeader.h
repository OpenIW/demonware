// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdMailHeader : public bdTaskResult
{
public:
    bdUInt64 m_senderID;
    bdNChar8 m_senderName[64];
    bdUInt64 m_mailID;
    bdUInt32 m_timeStamp;
    bdUInt m_size;
};
