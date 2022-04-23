// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#define BD_MAX_CHANNELS 5

enum bdLogMessageType
{
    BD_LOG_INFO = 0x0,
    BD_LOG_WARNING = 0x1,
    BD_LOG_ERROR = 0x2,
};

void bdLogMessage(bdLogMessageType type, const char* baseChannel, const char* channel, const char* file, const char* function, unsigned int line, const char* format, ...);

class bdLogSubscriber : public bdLinkable
{
public:
    const char* m_channels[BD_MAX_CHANNELS];

    bdLogSubscriber();
    bool addChannel(const char* channel);
    void logMessage(bdLogMessageType type, const char* channelName, const char* file, const char* function, unsigned int line, const char* msg);
    virtual void publish(bdLogMessageType type, const char* channelName, const char* file, const char* function, unsigned int line, const char* msg);
};

#define bdLogError(channel, Msg, ...) bdLogMessage(BD_LOG_ERROR, "err/", channel, __FILE__, __FUNCTION__, __LINE__, Msg, ##__VA_ARGS__)
#define bdLogWarn(channel, Msg, ...) bdLogMessage(BD_LOG_WARNING, "warn/", channel, __FILE__, __FUNCTION__, __LINE__, Msg, ##__VA_ARGS__)
#define bdLogInfo(channel, Msg, ...) bdLogMessage(BD_LOG_INFO, "info/", channel, __FILE__, __FUNCTION__, __LINE__, Msg, ##__VA_ARGS__)
