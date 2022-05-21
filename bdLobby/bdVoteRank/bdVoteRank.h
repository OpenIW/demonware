// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdVoteRank
{
protected:
    bdRemoteTaskManager* m_remoteTaskManager;
public:
    bdVoteRank(bdRemoteTaskManager* remoteTaskManager);
    bdRemoteTaskRef submitRating(bdRatingInfo* ratings, const bdUInt numRatings);
};