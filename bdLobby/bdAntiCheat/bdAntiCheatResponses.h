// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdAntiCheatResponses
{
public:
    bdAntiCheatResponses(bdAntiCheatResponses* other);
    bdAntiCheatResponses();
    ~bdAntiCheatResponses();
    bdBool serialize(bdByteBuffer* buffer);
    void clear();
    void addResponse(bdAntiCheatChallenge*, bdInt64);
    bdAntiCheatResponses* operator=(bdAntiCheatResponses* other);
private:
    bdFastArray<bdUInt64> m_challengeIDs;
    bdFastArray<bdInt64> m_responses;
    bdUInt16 m_numResponses;
};