// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdAddrHandleRefWrapper
{
protected:
    bdAddrHandleRef m_handle;
public:
    bdAddrHandleRefWrapper();
    bdAddrHandleRefWrapper(const bdAddrHandleRefWrapper* other);
    bdAddrHandleRefWrapper(bdAddrHandleRef handle);
    ~bdAddrHandleRefWrapper();
    bdBool operator==(const bdAddrHandleRefWrapper* other) const;
    bdUInt getHash(const bdAddrHandleRefWrapper* addr) const;
    bdAddrHandleRef getHandle() const;
};