// SPDX-License-Identifier: GPL-3.0-or-later

class bdServiceBandwidthArbitrator
{
protected:
    bdInt m_allowedSend;
    bdBool m_sentThisSlice;
public:
    bdServiceBandwidthArbitrator();

    void addSliceQuota(bdUInt bytes);
    bdBool allowedSend(bdUInt __formal);
    void reset();
    void send(bdUInt bytes);
};