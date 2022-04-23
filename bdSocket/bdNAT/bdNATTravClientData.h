// SPDX-License-Identifier: GPL-3.0-or-later

class bdNATTravClientData
{
public:
    enum bdNATTravClientDataState : bdInt
    {
        BD_NTCDS_INITIAL = 0x0,
        BD_NTCDS_STAGE_1 = 0x1,
        BD_NTCDS_STAGE_2 = 0x2,
        BD_NTCDS_STAGE_3 = 0x3,
    };
    bdNATTravClientDataState m_state;
    bdReference<bdCommonAddr> m_local;
    bdReference<bdCommonAddr> m_remote;
    bdNATTravListener* m_listener;
    bdNATTravListener* m_secondaryListener;
    unsigned int m_tries;
    bdStopwatch m_lastSent;
    bool m_throttled;
    bdStopwatch m_age;

    bdNATTravClientData();
    bdNATTravClientData(const bdNATTravClientData* other);
    bdNATTravClientData(bdCommonAddrRef local, bdCommonAddrRef remote, bdNATTravListener* listener);
    ~bdNATTravClientData();
    void callOnNATAddrDiscovery(bdCommonAddrRef remote, const bdAddr* realAddr);
    void callOnNATAddrDiscoveryFailed(bdCommonAddrRef remote);
    bdNATTravClientData* operator=(bdNATTravClientData* other);
};