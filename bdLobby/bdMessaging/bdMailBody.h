// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdMailBody : public bdTaskResult
{
public:
    void* m_mailBody;
    bdUInt32 m_size;
    bdUInt64 m_mailID;
};
