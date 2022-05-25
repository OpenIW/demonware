// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdPlatform/bdPlatform.h"

bdLogSubscriber* g_logSubscriberList;

void bdLogMessage(bdLogMessageType type, const bdNChar8* baseChannel, const bdNChar8* channel, const bdNChar8* file, const bdNChar8* function, bdUInt line, const bdNChar8* format, ...)
{
    va_list ap;
    bdLogSubscriber* i;
    bdNChar8 channelNameBuffer[256];
    bdNChar8 message[256];

    va_start(ap, format);
    if (g_logSubscriberList)
    {
        memset(message, 0, sizeof(message));
        vsnprintf_s(message, 0x100u, 0xFFFFFFFF, format, ap);
        if (_vscprintf(format, ap) >= 256)
        {
            bdLogWarn("bdPlatformLog", "Messaged truncated.");
        }
        if (baseChannel && channel)
        {
            if (bdSnprintf(channelNameBuffer, 0x100u, "%s%s", baseChannel, channel) >= 256)
            {
                bdLogWarn("bdPlatformLog", "Channel name truncated: %s%s", baseChannel, channel);
            }
            baseChannel = channelNameBuffer;
        }
        for (i = g_logSubscriberList; i; i = (bdLogSubscriber*)i->getNext())
        {
            i->logMessage(type, baseChannel, file, function, line, message);
        }
    }
}

bdLogSubscriber::bdLogSubscriber()
{
    memset(m_channels, 0, sizeof(m_channels));
}

bool bdLogSubscriber::addChannel(const bdNChar8* channel)
{
    bdBool found = false;

    for (bdUInt i = 0; !found && i < BD_MAX_CHANNELS; ++i)
    {
        if (!m_channels[i])
        {
            found = true;
            m_channels[i] = channel;
        }
    }
    if (found && !bdLinkable::find(g_logSubscriberList, this))
    {
        link((bdLinkable**)&g_logSubscriberList);
    }
    return found;
}

void bdLogSubscriber::logMessage(bdLogMessageType type, const bdNChar8* channelName, const bdNChar8* file, const bdNChar8* function, bdUInt line, const bdNChar8* msg)
{
    for (bdUInt i = 0; i < BD_MAX_CHANNELS; ++i)
    {
        if (m_channels[i])
        {
            if (bdDelimSubstr(channelName, m_channels[i], "\\/"))
            {
                publish(type, channelName, file, function, line, msg);
            }
        }
    }
}

void bdLogSubscriber::publish(bdLogMessageType type, const bdNChar8* channelName, const bdNChar8* file, const bdNChar8* function, bdUInt line, const bdNChar8* msg)
{
    const bdNChar8* logType;
    const bdNChar8* fileName;

    switch (type)
    {
    case BD_LOG_INFO:
        logType = " ";
        break;
    case BD_LOG_WARNING:
        logType = " WARNING: ";
        break;
    case BD_LOG_ERROR:
        logType = " ERROR: ";
        break;
    default:
        logType = " ";
        break;
    }
    fileName = strrchr(file, '\\');
    if (fileName)
    {
        bdPrintf("%s(%u):%s%s\n", fileName + 1, line, logType, msg);
    }
    else
    {
        bdPrintf("%s(%u):%s%s\n", file, line, logType, msg);
    }
}
