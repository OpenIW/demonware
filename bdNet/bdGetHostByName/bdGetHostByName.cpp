// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdNet/bdNet.h"

void* bdGetHostByName::operator new(bdUWord nbytes)
{
    return bdMemory::allocate(nbytes);
}

void bdGetHostByName::operator delete(void* p)
{
    bdMemory::deallocate(p);
}

bdGetHostByName::~bdGetHostByName()
{
    quit();
}

bdGetHostByName::bdGetHostByName()
    : bdRunnable(), m_timer(), m_status(BD_LOOKUP_UNINITIALIZED), m_thread(NULL), m_config(), m_numAddresses(0), m_addresses()
{
}

bdBool bdGetHostByName::start(const bdNChar8* hostname, bdGetHostByNameConfig config)
{
    if (m_status)
    {
        bdLogWarn("gethostbyname", "Cannot start a host name lookup unless class is in the uninitialized state.");
        return false;
    }
    m_thread = new bdThread(this, 0, 0);
    m_config.m_timeout = config.m_timeout;
    m_timer.reset();
    m_timer.start();
    config.sanityCheckConfig();
    m_status = BD_LOOKUP_PENDING;
    bdBool success = m_thread->start(reinterpret_cast<const void*>(hostname), bdStrlen(hostname) + 1);
    if (!success)
    {
        m_status = BD_LOOKUP_ERROR;
        bdLogError("gethostbyname", "Failed to start host name lookup thread for %s.", hostname);
    }
    bdLogInfo("gethostbyname", "Lookup started for %s.", hostname);
    return success;
}

bdUInt bdGetHostByName::run(void* args)
{
    bdInt poolID;

    if (m_status != BD_LOOKUP_PENDING)
    {
        bdLogWarn("gethostbyname", "Host name lookup thread cannot be started unless a request is pending.");
        return 0;
    }
    m_numAddresses = bdPlatformSocket::getHostByName(reinterpret_cast<char*>(args), m_addresses, 4u);
    if (m_numAddresses)
        m_status = BD_LOOKUP_SUCCEEDED;
    else
        m_status = BD_LOOKUP_FAILED;
    return 0;
}

void bdGetHostByName::quit()
{
    if (!m_status)
    {
        return;
    }
    if (m_status == BD_LOOKUP_PENDING)
    {
        cancelLookup();
    }
    if (m_thread)
    {
        m_thread->join();
        m_thread->cleanup();
    }
}

void bdGetHostByName::pump()
{
    if (m_status != BD_LOOKUP_PENDING)
    {
        bdLogInfo("gethostbyname", "Pumping class while no active lookup. This is perfectly safe but not necessary.");
        return;
    }
    if (m_config.m_timeout < m_timer.getElapsedTimeInSeconds())
    {
        bdLogError("gethostbyname", "Lookup has timed out after %f seconds, cancelling thread.", m_config.m_timeout);
        cancelLookup();
        m_status = BD_LOOKUP_TIMED_OUT;
    }
}

void bdGetHostByName::cancelLookup()
{
    if (m_status != BD_LOOKUP_PENDING)
    {
        bdLogWarn("gethostbyname", "No pending lookup to cancel");
        return;
    }
    m_status = BD_LOOKUP_CANCELLED;
}

bdInAddr bdGetHostByName::getAddressAt(bdUInt index) const
{
    if (index < m_numAddresses)
    {
        return bdInAddr(m_addresses[index]);
    }
    if (index >= 4)
    {
        bdLogWarn("gethostbyname", "Address at index %u is outside the array, blank bdInAddr returned");
    }
    else
    {
        bdLogWarn("gethostbyname", "No address at index %u , blank bdInAddr returned");
    }
    return bdInAddr(m_addresses[index]);
}

const bdUInt bdGetHostByName::getNumAddresses() const
{
    return m_numAddresses;
}

const bdGetHostByName::bdStatus bdGetHostByName::getStatus() const
{
    return m_status;
}
