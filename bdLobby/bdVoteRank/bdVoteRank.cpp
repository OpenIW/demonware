// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdLobby/bdLobby.h"

bdVoteRank::bdVoteRank(bdRemoteTaskManager* remoteTaskManager)
    : m_remoteTaskManager(remoteTaskManager)
{
}

bdVoteRank::~bdVoteRank()
{
}

bdRemoteTaskRef bdVoteRank::submitRating(bdRatingInfo* ratings, const bdUInt numRatings)
{
    bdRemoteTaskRef task;
    bdUInt taskSize = 64;
    for (bdUInt i = 0; i < numRatings; ++i)
    {
        taskSize += ratings[i].sizeOf();
    }
    bdTaskByteBufferRef buffer(new bdTaskByteBuffer(taskSize, true));
    m_remoteTaskManager->initTaskBuffer(buffer, 55, 1);
    for (bdUInt i = 0; i < numRatings; ++i)
    {
        ratings[i].serialize(**buffer);
    }
    m_remoteTaskManager->startTask(task, buffer);
    return task;
}
