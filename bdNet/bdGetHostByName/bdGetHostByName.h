// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdGetHostByName : public bdRunnable
{
public:
    enum bdStatus : bdInt
    {
        BD_LOOKUP_UNINITIALIZED = 0x0,
        BD_LOOKUP_PENDING = 0x1,
        BD_LOOKUP_SUCCEEDED = 0x2,
        BD_LOOKUP_FAILED = 0x3,
        BD_LOOKUP_CANCELLED = 0x4,
        BD_LOOKUP_TIMED_OUT = 0x5,
        BD_LOOKUP_ERROR = 0x6,
    };
protected:
    bdStopwatch m_timer;
    bdStatus m_status;
    bdThread* m_thread;
    bdGetHostByNameConfig m_config;
    bdInAddr m_addresses[4];
    bdUInt m_numAddresses;
    bdDNSHandle m_handle;
public:
    void* operator new(bdUWord nbytes);
    void operator delete(void* p);
    bdGetHostByName();
    ~bdGetHostByName();
    bdBool start(const bdNChar8* hostname, bdGetHostByNameConfig config);
    virtual bdUInt run(void* args);
    void quit();
    void pump();
    void cancelLookup();

    bdInAddr* getAddressAt(bdUInt index) const;
    const bdUInt getNumAddresses() const;
    const bdGetHostByName::bdStatus getStatus() const;
};