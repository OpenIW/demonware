// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define BD_MAX_CHANNELS 5

enum bdLogMessageType
{
    BD_LOG_INFO = 0x0,
    BD_LOG_WARNING = 0x1,
    BD_LOG_ERROR = 0x2,
};

void bdLogMessage(bdLogMessageType type, const bdNChar8* baseChannel, const bdNChar8* channel, const bdNChar8* file, const bdNChar8* function, bdUInt line, const bdNChar8* format, ...);

class bdLogSubscriber : public bdLinkable
{
public:
    const bdNChar8* m_channels[BD_MAX_CHANNELS];

    bdLogSubscriber();
    bool addChannel(const bdNChar8* channel);
    void logMessage(bdLogMessageType type, const bdNChar8* channelName, const bdNChar8* file, const bdNChar8* function, bdUInt line, const bdNChar8* msg);
    virtual void publish(bdLogMessageType type, const bdNChar8* channelName, const bdNChar8* file, const bdNChar8* function, bdUInt line, const bdNChar8* msg);
};

#define bdLogError(channel, Msg, ...) bdLogMessage(BD_LOG_ERROR, "err/", channel, __FILE__, __FUNCTION__, __LINE__, Msg, ##__VA_ARGS__)
#define bdLogWarn(channel, Msg, ...) bdLogMessage(BD_LOG_WARNING, "warn/", channel, __FILE__, __FUNCTION__, __LINE__, Msg, ##__VA_ARGS__)
#define bdLogInfo(channel, Msg, ...) bdLogMessage(BD_LOG_INFO, "info/", channel, __FILE__, __FUNCTION__, __LINE__, Msg, ##__VA_ARGS__)
