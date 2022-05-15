// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

struct bdHTTPBuffer
{
    void* m_contentReceiveBuffer;
    bdUInt m_contentReceiveBufferSize;
    const void* m_contentSendBuffer;
    bdUInt m_contentSendBufferSize;
    bdNChar8 m_httpCommonBuffer[1024];
    bdUInt m_httpCommonBufferSize;
    bdNChar8 m_responseStatusBuffer[128];
    bdUInt m_responseStatusBufferSize;
};
