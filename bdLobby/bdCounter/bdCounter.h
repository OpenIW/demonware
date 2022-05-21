// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdCounter
{
protected:
    bdRemoteTaskManager* m_remoteTaskManager;
public:
    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdCounter(bdRemoteTaskManager* const remoteTaskManager);
    bdRemoteTaskRef incrementCounters(bdCounterValue* const counterIncrements, const bdUInt numEntries);
    bdRemoteTaskRef getCounterTotals(bdCounterValue* results, const bdUInt numCounterIDs);
};