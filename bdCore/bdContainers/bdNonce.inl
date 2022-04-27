// SPDX-License-Identifier: GPL-3.0-or-later

template<bdInt SIZE>
inline bdNonce<SIZE>::bdNonce()
{
    m_initialised = false;
}

template<bdInt SIZE>
inline void bdNonce<SIZE>::ensureCreated()
{
    if (!m_initialised)
    {
        bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUByte8(reinterpret_cast<bdUByte8*>(m_nonce), SIZE);
        m_initialised = true;
    }
}

template<bdInt SIZE>
inline const bdUByte8* bdNonce<SIZE>::getData()
{
    //bdAssertMsg(m_initialised, "Attempted to retrieve a nonce without initialising it first");
    return m_nonce;
}

