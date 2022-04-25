// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

const class bdQoSProbeInfo
{
public:
    bdCommonAddrRef m_addr;
    bdAddr m_realAddr;
    bdFloat32 m_latency;
    bdUByte8* m_data;
    bdUInt m_dataSize;
    bdBool m_disabled;
    bdUInt m_bandwidthDown;
    bdUInt m_bandwidthUp;

    bdQoSProbeInfo() : m_addr(), m_realAddr(), m_latency(0), m_data(NULL), m_dataSize(0), m_disabled(false), m_bandwidthDown(0), m_bandwidthUp(0) {};
    ~bdQoSProbeInfo() {};
};