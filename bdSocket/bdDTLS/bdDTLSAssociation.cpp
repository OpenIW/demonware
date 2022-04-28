// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdSocket/bdSocket.h"

bdDTLSAssociation::bdDTLSAssociation(bdSocket* socket, bdSecurityKeyMap* keyMap, bdECCKey* ECCKey, bdAddr* addr, bdAddrHandleRef addrHandle, bdCommonAddrRef localCommonAddr, bdAddressMap* addrMap,
    bdFloat32 receiveTimeout)
    : m_socket(socket), m_keyMap(keyMap), m_ECCKey(ECCKey), m_cypher(), m_addr(addr), m_addrHandle(addrHandle), m_state(BD_DTLS_CLOSED), m_lastReceived(), m_initTimer(), m_initResends(0),
    m_cookieTimer(), m_cookieResends(0), m_seqNum(-1), m_incomingSeqNums(&bdSequenceNumber()), m_initAck(), m_localId(), m_localCommonAddr(localCommonAddr), m_addrMap(addrMap), m_receiveTimeout(receiveTimeout)
{
    reset();
    if (!m_cookieInited)
    {
        bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUByte8(m_cookieKey, sizeof(m_cookieKey));
        m_cookieInited = true;
    }
}

bdDTLSAssociation::~bdDTLSAssociation()
{
    m_ECCKey = NULL;
    m_keyMap = NULL;
    m_socket = NULL;
}

void bdDTLSAssociation::connect()
{
    sendInit();
    m_state = BD_DTLS_COOKIE_WAIT;
}

void bdDTLSAssociation::pump()
{
    bdNChar8 addrString[22];

    switch (m_addrHandle->getStatus())
    {

    case bdAddrHandle::BD_ADDR_NOT_CONSTRUCTED:
    {
        m_addrHandle->getRealAddr()->toString(addrString, sizeof(addrString));
        bdLogWarn("bdSocket/dtls", "Address not constructed (%s). Shouldn't be here. Closing.", addrString);
        m_state = BD_DTLS_CLOSED;
        break;
    }
    case bdAddrHandle::BD_ADDR_NOT_RESOLVED:
    {
        m_addrHandle->getRealAddr()->toString(addrString, sizeof(addrString));
        bdLogWarn("bdSocket/dtls", "Address not resolved (%s). Shouldn't be here. Closing.", addrString);
        m_state = BD_DTLS_CLOSED;
        break;
    }
    case bdAddrHandle::BD_ADDR_ERROR:
    {
        bdLogWarn("bdSocket/dtls", "Assress error.");
        m_state = BD_DTLS_CLOSED;
        break;
    }
    case bdAddrHandle::BD_ADDR_UNRESOLVED:
    {
        bdLogWarn("bdSocket/dtls", "Address unregistered. Closing DTLS.");
        m_state = BD_DTLS_CLOSED;
        break;
    }
    default:
        break;

    }

    if (m_lastReceived.getElapsedTimeInSeconds() > m_receiveTimeout)
    {
        m_addrHandle->getRealAddr()->toString(addrString, sizeof(addrString));
        bdLogInfo("bdSocket/dtls", "DTLS receive timeout. Closing connection to %s", addrString);
        m_state = BD_DTLS_CLOSED;
        return;
    }
    switch (m_state)
    {
    case BD_DTLS_COOKIE_WAIT:
    {
        if (m_initTimer.getElapsedTimeInSeconds() > 1.0)
        {
            sendInit();
        }
        break;
    }
    case BD_DTLS_COOKIE_ECHOED:
    {
        if (m_cookieTimer.getElapsedTimeInSeconds() > 1.0)
        {
            sendCookieEcho(&m_addr);
        }
        break;
    }
    default:
        break;
    }
}

void bdDTLSAssociation::reset()
{
    m_initResends = 0;
    m_cookieResends = 0;
    m_seqNum = bdSequenceNumber(0);
    m_localTag = bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUInt();
    m_lastReceived.start();
    m_peerTag = 0;
    m_incomingSeqNums = bdSequenceNumberStore(&bdSequenceNumber(-1));
}

bdBool bdDTLSAssociation::verify(bdDTLSData* dataPacket)
{
    bdSequenceNumber seqNum(-1);
    if (dataPacket->getVtag() != m_localTag)
    {
        bdLogWarn("bdSocket/dtls", "vtag not the same as local tag.");
        return false;
    }
    bdSequenceNumber newSeqNum(m_incomingSeqNums.getLastSequenceNumber(), dataPacket->getCounter(), 16u);
    switch (m_incomingSeqNums.check(&newSeqNum))
    {
    case bdSequenceNumberStore::BD_SN_INVALID_SMALLER:
        bdLogWarn("bdSocket/dtls", "Invalid seq num (%u) is smaller than last (%u).", newSeqNum.getValue(), m_incomingSeqNums.getLastSequenceNumber()->getValue());
        return false;
    case bdSequenceNumberStore::BD_SN_VALID_MUCH_LARGER:
        return true;
    default:
        bdLogWarn("bdSocket/dtls", "Invalid seq num (%u) is a duplicate of the last (%u).", newSeqNum.getValue(), m_incomingSeqNums.getLastSequenceNumber()->getValue());
        return false;
    }
}

bdBool bdDTLSAssociation::calculateSharedKey(const bdUByte8* const pubKey, const bdUInt keylen, const bdSecurityID* secID)
{
    bdUByte8 sharedSecret[44];
    bdUInt32 sharedKeyAsUInt[6];

    bdSecurityKey securityKey;
    if (!m_keyMap->get(secID, &securityKey))
    {
        bdLogWarn("bdSocket/dtls", "secid not in keymap.");
        return false;
    }
    if (!m_ECCKey->generateSharedSecret(pubKey, keylen, sharedSecret, sizeof(sharedSecret)))
    {
        bdLogWarn("bdSocket/dtls", "couldn't generate shared key.");
        return false;
    }
    bdMemcpy(&sharedSecret[28], &securityKey, sizeof(sharedSecret) - 28);
    bdHashTiger192 tiger;
    bdUInt hashSize = 24;
    if (!tiger.hash(sharedSecret, sizeof(sharedSecret), m_sharedKey, &hashSize))
    {
        bdLogWarn("bdSocket/dtls", "unable to create hash.");
        return false;
    }
    m_cypher.init(m_sharedKey, sizeof(m_sharedKey));
    bdMemcpy(sharedKeyAsUInt, this->m_sharedKey, sizeof(sharedKeyAsUInt));
    bdLogInfo("bdSocket/dtls", "Shared key: %08x %08x %08x %08x %08x %08x", sharedKeyAsUInt[0], sharedKeyAsUInt[1], sharedKeyAsUInt[2], sharedKeyAsUInt[3], sharedKeyAsUInt[4], sharedKeyAsUInt[5]);
    return true;
}

bdInt bdDTLSAssociation::sendTo(const bdAddr* addr, const void* data, const bdUInt length, const bdSecurityID* secID)
{
    bdInt val = -1;
    if (m_state == BD_DTLS_ESTABLISHED)
    {
        return sendData(addr, data, length, secID);
    }
    if (m_state <= BD_DTLS_COOKIE_ECHOED)
    {
        bdLogWarn("bdSocket/dtls", "not connected. Cannot send.");
    }
    return val;
}

bdBool bdDTLSAssociation::checkCookieValidity(const bdAddr* peerAddr, const bdDTLSCookieEcho* cookiePacket)
{
    bdUInt age = bdPlatformTiming::getLoResElapsedTime(cookiePacket->getCookie()->getTimestamp(), GetTickCount());
    if (age >= 60)
    {
        bdLogWarn("bdSocket/dtls", "Cookie expired. Age: %us", age);
        return false;
    }
    if (cookiePacket->getCookie()->getPeerAddr() != peerAddr)
    {
        bdLogWarn("bdSocket/dtls", "Wrong peer address");
        return false;
    }
    bdHMacSHA1 hmac(m_cookieKey, sizeof(m_cookieKey));
    if (!const_cast<bdDTLSInitAck*>(cookiePacket->getCookie())->verify(&hmac))
    {
        bdLogWarn("bdSocket/dtls", "HMac Verification failed.");
        return false;
    }
    return true;
}

bdInt bdDTLSAssociation::receiveFrom(bdAddr* addr, const void* data, const bdUInt size, bdAddrHandleRef* addrHandle, void* buffer, const bdUInt bufferSize)
{
    bdUInt tmpUInt;
    bdInt val = -2;
    bdDTLSHeader header;

    if (header.deserialize(data, size, 0, &tmpUInt))
    {
        switch (header.getType())
        {
        case 1:
            val = handleInit(addr, data, size);
            break;
        case 2:
            val = handleInitAck(addr, data, size);
            break;
        case 3:
            val = handleCookieEcho(addr, data, size);
            break;
        case 4:
            val = handleCookieAck(data, size);
            break;
        case 5:
            val = handleError(addr, data, size);
            break;
        case 6:
            val = handleData(addr, reinterpret_cast<const bdUByte8*>(data), size, addrHandle, reinterpret_cast<bdUByte8*>(buffer), bufferSize);
            break;
        default:
            val = -2;
            break;
        }
    }
    return val;
}

bdInt bdDTLSAssociation::handleInit(const bdAddr* addr, const void* data, const bdUInt size)
{
    bdUInt tempUInt;
    bdDTLSInit init;

    if (!init.deserialize(data, size, 0, &tempUInt))
    {
        bdLogWarn("bdSocket/dtls", "bad init packet.");
    }
    else
    {
        bdLogInfo("bdSocket/dtls", "handling init: m_localTag: %X, init->m_initTag: %X");
        sendInitAck(addr, &init);
        m_lastReceived.start();
    }
    return -2;
}

bdInt bdDTLSAssociation::handleInitAck(const bdAddr* addr, const void* data, const bdUInt size)
{
    bdUInt tmpUInt;

    if (m_state != BD_DTLS_COOKIE_WAIT)
    {
        return -2;
    }
    bdDTLSInitAck initAck;
    if (!initAck.deserialize(data, size, 0, &tmpUInt))
    {
        bdLogWarn("bdSocket/dtls", "Failed to deserialize init ack. Ignoring");
        return -2;
    }
    m_initAck = &initAck;
    if (m_initAck.getVtag() != m_localTag)
    {
        bdLogWarn("bdSocket/dtls", "Received init ack with invalid vtag. Ignoring");
        return -2;
    }
    m_peerTag = m_initAck.getInitTag();
    bdLogInfo("bdSocket/dtls", "handling init ack: m_localTag/m_peerTag: %X/%X", m_localTag, m_peerTag);
    sendCookieEcho(addr);
    m_lastReceived.start();
    m_state = BD_DTLS_COOKIE_ECHOED;
    return -2;
}

bdInt bdDTLSAssociation::handleCookieEcho(const bdAddr* addr, const void* data, const bdUInt size)
{
    bdDTLSCookieEcho cookieEcho;
    bdUInt tmpUInt;
    bdDTLSError::bdDTLSErrorType error;

    if (!cookieEcho.deserialize(data, size, 0, &tmpUInt))
    {
        bdLogWarn("bdSocket/dtls", "Failed to deserialize cookie echo. Ignoring");
        return -2;
    }
    if (!checkCookieValidity(addr, &cookieEcho))
    {
        bdLogWarn("bdSocket/dtls", "Received bad cookie. Ignoring.");
        return -2;
    }
    bdUInt16 localTag = cookieEcho.getCookie()->getLocalTag();
    bdUInt16 peerTag = cookieEcho.getCookie()->getPeerTag();
    bdUInt16 localTieTag = cookieEcho.getCookie()->getLocalTieTag();
    bdUInt16 peerTieTag = cookieEcho.getCookie()->getPeerTieTag();

    bdSecurityID secID;
    bdMemcpy(&secID, cookieEcho.getSecID(), sizeof(secID));

    bdCommonAddr* p = new bdCommonAddr();
    bdCommonAddrRef commonAddr(p);

    if (!commonAddr->deserialize(bdCommonAddrRef(&m_localCommonAddr), cookieEcho.getCa()))
    {
        return -2;
    }

    if (m_state)
    {
        if (m_localTag == localTag || m_peerTag == peerTag || m_localTag != localTieTag || m_peerTag != peerTieTag)
        {
            if ((m_localTag != localTag || m_peerTag == peerTag) && (m_localTag != localTag || peerTieTag))
            {
                if (m_localTag == localTag || m_peerTag != peerTag)
                {
                    if (m_localTag == localTag && m_peerTag == peerTag)
                    {
                        if (m_localId != &secID)
                        {
                            bdLogWarn("bdSocket/dtls", "Received cookie echo in state D, but local secID doesn't match the cookie one. This connection is unlikely to work.");
                        }
                        m_state = BD_DTLS_ESTABLISHED;
                        bdLogInfo("bdSocket/dtls", "handling cookie echo (D): m_localTag/m_peerTag: %X/%X", m_localTag, m_peerTag);
                        sendCookieAck(addr, &cookieEcho);
                        m_localId = secID;
                    }
                    else
                    {
                        bdLogInfo("bdSocket/dtls", "handling cookie echo (E): m_localTag/m_peerTag: %X/%X", m_localTag, m_peerTag);
                    }
                }
                else
                {
                    bdLogInfo("bdSocket/dtls", "handling cookie echo (C): m_localTag/m_peerTag: %X/%X", m_localTag, m_peerTag);
                }
            }
            else
            {
                if (m_addrHandle->getStatus() != bdAddrHandle::BD_ADDR_RESOLVED)
                {
                    bdLogWarn("bdSocket/dtls", "Invalid addr handle!");
                }
                m_peerTag = peerTag;
                bdLogInfo("bdSocket/dtls", "handling cookie echo (B): m_localTag/m_peerTag: %X/%X", m_localTag, m_peerTag);
                if (!m_keyMap->contains(&secID))
                {
                    bdLogWarn("bdSocket/dtls", "secID not in keymap.");
                    error = bdDTLSError::BD_DTLS_ERROR_BAD_SECID;
                    sendError(addr, &secID, &error);
                    m_state = BD_DTLS_CLOSED;
                    m_lastReceived.start();
                    return -2;
                }
                if (!calculateSharedKey(cookieEcho.getECCKey(), 0x64u, &secID))
                {
                    bdLogWarn("bdSocket/dtls", "Failed to generate shared secret.");
                    return -2;
                }
                m_addrMap->getAddrHandle(bdCommonAddrRef(&commonAddr), &secID, &m_addrHandle);
                m_addrHandle->setRealAddr(addr);
                bdMemcpy(&m_addr, addr, sizeof(addr));
                sendCookieAck(addr, &cookieEcho);
                m_state = BD_DTLS_ESTABLISHED;
                bdLogInfo("bdSocket/dtls", "DTLS established.");
                m_localId = secID;
            }
        }
        else if (m_keyMap->contains(&secID))
        {
            if (!calculateSharedKey(cookieEcho.getECCKey(), 0x64, &secID))
            {
                bdLogWarn("bdSocket/dtls", "Failed to generate shared secret");
                return -2;
            }
            reset();
            m_localTag = localTag;
            m_peerTag = peerTag;
            m_addrMap->getAddrHandle(bdCommonAddrRef(&commonAddr), &secID, &m_addrHandle);
            m_addrHandle->setRealAddr(addr);
            bdMemcpy(&m_addr, addr, sizeof(addr));
            sendCookieAck(addr, &cookieEcho);
            bdLogInfo("bdSocket/dtls", "handling cookie echo (A): m_localTag/m_peerTag: %X/%X", m_localTag, m_peerTag);
            m_state = BD_DTLS_ESTABLISHED;
            bdLogInfo("bdSocket/dtls", "DTLS established.");
            m_localId = secID;
        }
        else
        {
            bdLogWarn("bdSocket/dtls", "secID not in keymap.");
            error = bdDTLSError::BD_DTLS_ERROR_BAD_SECID;
            sendError(addr, &secID, &error);
            m_state = BD_DTLS_CLOSED;
        }
    }
    else
    {
        m_localTag = localTag;
        m_peerTag = peerTag;
        if (!m_keyMap->contains(&secID))
        {
            bdLogWarn("bdSocket/dtls", "secID not in keymap.");
            error = bdDTLSError::BD_DTLS_ERROR_BAD_SECID;
            sendError(addr, &secID, &error);
            m_state = BD_DTLS_CLOSED;
            m_lastReceived.start();
            return -2;
        }
        if (!calculateSharedKey(cookieEcho.getECCKey(), 0x64u, &secID))
        {
            bdLogWarn("bdSocket/dtls", "Failed to generate shared secret");
            return -2;
        }
        if (!m_addrMap->getAddrHandle(bdCommonAddrRef(&commonAddr), &secID, &m_addrHandle))
        {
            return -2;
        }
        m_addrHandle->setRealAddr(addr);
        bdMemcpy(&m_addr, addr, sizeof(addr));
        bdLogInfo("bdSocket/dtls", "handling cookie echo (expected): m_localTag/m_peerTag: %X/%X", m_localTag, m_peerTag);
        sendCookieAck(addr, &cookieEcho);
        m_state = BD_DTLS_ESTABLISHED;
        bdLogInfo("bdSocket/dtls", "DTLS established.");
        m_localId = secID;
    }

    //ends
    m_lastReceived.start();
    return -2;
}

bdInt bdDTLSAssociation::handleCookieAck(const void* data, const bdUInt size)
{
    bdNChar8 addrBuffer[22];
    bdUInt tmpUInt;
    bdDTLSCookieAck cookieAck;

    if (!cookieAck.deserialize(data, size, 0, &tmpUInt))
    {
        bdLogWarn("bdSocket/dtls", "Failed to deserialize cookie ack. Ignoring");
        return -2;
    }
    if (cookieAck.getVtag() != m_localTag)
    {
        bdLogWarn("bdSocket/dtls", "Received cookie ack with invalid vtag. Ignoring");
        return -2;
    }
    bdSecurityID secID;
    cookieAck.getSecID(&secID);
    if (m_state == BD_DTLS_ESTABLISHED)
    {
        bdLogInfo("bdSocket/dtls", "Received cookie ack for already established DTLS, ignoring.");
    }
    else if (m_state == BD_DTLS_COOKIE_ECHOED)
    {
        if (!calculateSharedKey(cookieAck.getECCKey(), 0x64u, &secID))
        {
            m_addr.toString(addrBuffer, sizeof(addrBuffer));
            bdLogWarn("bdSocket/dtls", "Failed to calculate shared key for %s ", addrBuffer);
            return -2;
        }
        m_lastReceived.start();
        m_state = BD_DTLS_ESTABLISHED;
        bdLogInfo("bdSocket/dtls", "handled cookie ack. DTLS established.");
        m_localId = secID;
    }
    else if (m_state <= BD_DTLS_COOKIE_WAIT)
    {
        bdLogInfo("bdSocket/dtls", "Received cookie ack when in unexpected state %u. ", m_state);
    }
    return -2;
}

bdInt bdDTLSAssociation::handleError(const bdAddr* addr, const void* data, const bdUInt size)
{
    bdUInt tmpUInt;
    bdNChar8 addrStr[22];
    bdDTLSError error;

    if (error.deserialize(data, size, 0, &tmpUInt) && error.getVtag() == m_localTag)
    {
        addr->toString(addrStr, sizeof(addrStr));
        if (error.getEtype())
        {
            bdLogWarn("bdSocket/dtls", "Received unrecognized error message from %s. Ignoring.", addrStr);
        }
        else
        {
            bdLogInfo("bdSocket/dtls", "Received bad_security_id error reset from %s. Closing the association.", addrStr);
            m_state = BD_DTLS_CLOSED;
        }
    }
    return -1;
}

bdInt bdDTLSAssociation::handleData(bdAddr* addr, const bdUByte8* data, const bdUInt size, bdAddrHandleRef* addrHandle, bdUByte8* buffer, const bdUInt bufferSize)
{
    bdUInt payloadSize;
    bdUInt dataHeaderSize;
    bdUInt val = -2;

    if (m_state != BD_DTLS_ESTABLISHED)
    {
        return val;
    }
    if (m_addrHandle->m_endpoint.getCommonAddr().isNull())
    {
        bdLogWarn("bdSocket/dtls", "Receiving data before m_addrHandle is set!!");
        return val;
    }
    bdDTLSData dataPacket;
    if (!dataPacket.deserialize(data, size, 0, &dataHeaderSize, m_incomingSeqNums.getLastSequenceNumber(), m_sharedKey, buffer, bufferSize, &payloadSize, &m_cypher, 
        &bdSecurityID(m_addrHandle->m_endpoint.getSecID())))
    {
        bdLogWarn("bdSocket/dtls", "Received data but deserialization failed!!");
        return val;
    }
    if (!verify(&dataPacket))
    {
        return val;
    }
    val = payloadSize;
    bdMemcpy(addr, &m_addr, sizeof(addr));
    addrHandle = &m_addrHandle;
    m_lastReceived.start();
    return val;
}

void bdDTLSAssociation::sendInit()
{
    bdUByte8 buffer[1288];
    bdUInt tmpUInt;

    if (m_initResends++ > 5.0f)
    {
        bdLogInfo("bdSocket/dtls", "sending init: too many retries, closing");
        m_state = BD_DTLS_CLOSED;
        return;
    }
    bdDTLSInit init(m_localTag, &bdSecurityID(m_addrHandle->m_endpoint.getSecID()));
    init.serialize(buffer, sizeof(buffer), 0, &tmpUInt);
    m_socket->sendTo(&m_addr, buffer, tmpUInt);
    m_initTimer.start();
    bdLogInfo("bdSocket/dtls", "sending init: m_localTag: %X", m_localTag);
}

void bdDTLSAssociation::sendInitAck(const bdAddr* addr, const bdDTLSInit* init)
{
    bdUInt16 localTieTag;
    bdUInt16 peerTieTag;
    bdUInt16 localTag;
    bdUInt16 peerTag;
    bdUInt tmpUInt;
    bdUByte8 buffer[1288];

    peerTag = init->getInitTag();
    localTag = 0;
    peerTieTag = 0;
    localTieTag = 0;
    switch (m_state)
    {
    case 0:
        m_peerTag = peerTag;
        break;
    case 1:
        localTag = m_localTag;
        break;
    case 2:
        peerTieTag = m_peerTag;
        localTieTag = m_localTag;
        localTag = m_localTag;
        break;
    case 3:
        localTag = bdSingleton<bdTrulyRandomImpl>::getInstance()->getRandomUInt();
        peerTieTag = m_peerTag;
        localTieTag = m_localTag;
        break;
    default:
        return;
    }

    bdSecurityID secID;
    init->getSecID(&secID);
    bdDTLSInitAck initAck(peerTag, localTag, localTag, peerTag, localTieTag, peerTieTag, GetTickCount(), addr, bdSecurityID(&secID));
    bdHMacSHA1 hmac(m_cookieKey, sizeof(m_cookieKey));
    initAck.sign(&hmac);
    initAck.serialize(buffer, sizeof(buffer), 0, &tmpUInt);
    // TEMP CONST_CAST
    bdUInt sentResult = m_socket->sendTo(const_cast<bdAddr*>(addr), buffer, tmpUInt);
    if (sentResult != tmpUInt)
    {
        bdLogWarn("bdSocket/dtls", "problem with socket?");
    }
    bdLogInfo("bdSocket/dtls", "sending init ack: m_localTag/localTag/m_peerTag: %d/%d/%d", m_localTag, localTag, m_peerTag);
}

void bdDTLSAssociation::sendCookieEcho(const bdAddr* addr)
{
    bdUByte8 buffer[1288];
    bdUInt tmpUInt;

    if (m_cookieResends++ > 5.0)
    {
        bdLogInfo("bdSocket/dtls", "sending cookie echo: too many retries, closing");
        m_state = BD_DTLS_CLOSED;
        return;
    }
    bdDTLSCookieEcho cookieEcho(m_peerTag, &m_initAck, bdCommonAddrRef(&m_localCommonAddr), m_ECCKey);
    cookieEcho.serialize(buffer, sizeof(buffer), 0, &tmpUInt);
    if (m_socket->sendTo(const_cast<bdAddr*>(addr), buffer, tmpUInt) < 0)
    {
        bdLogError("bdSocket/dtls", "Failed to send cookie echo");
    }
    else
    {
        bdLogInfo("bdSocket/dtls", "sending cookie echo: m_localTag/m_peerTag: %X/%X");
        m_state = BD_DTLS_COOKIE_ECHOED;
    }
    m_cookieTimer.start();
}

void bdDTLSAssociation::sendCookieAck(const bdAddr* addr, const bdDTLSCookieEcho* cookie)
{
    bdUByte8 buffer[1288];
    bdUInt tmpUInt;
    bdUInt keyLen;
    bdUByte8 key[100];

    keyLen = 100;
    if (!m_ECCKey->exportKey(key, &keyLen) || keyLen != 100)
    {
        bdLogWarn("bdSocket/dtls", "problem with dh key");
    }
    bdDTLSCookieAck cookieAck(m_peerTag, key, bdSecurityID(m_addrHandle->m_endpoint.getSecID()));
    cookieAck.serialize(buffer, sizeof(buffer), 0, &tmpUInt);
    m_socket->sendTo(const_cast<bdAddr*>(addr), buffer, tmpUInt);
    bdLogInfo("bdSocket/dtls", "sending cookie ack: m_localTag/m_peerTag: %X/%X", m_localTag, m_peerTag);
}

void bdDTLSAssociation::sendError(const bdAddr* addr, const bdSecurityID* secID, const bdDTLSError::bdDTLSErrorType* type)
{
    bdUInt tmpUInt;
    bdUByte8 buffer[1288];

    bdDTLSError error(m_peerTag, *type, secID);
    error.serialize(buffer, sizeof(buffer), 0, &tmpUInt);
    m_socket->sendTo(const_cast<bdAddr*>(addr), buffer, tmpUInt);
    bdLogInfo("bdSocket/dtls", "sending error: etype: %d", *type);
}

bdInt bdDTLSAssociation::sendData(const bdAddr* addr, const void* data, const bdUInt length, const bdSecurityID* secID)
{
    bdUInt packetLength;
    bdUByte8 outData[1288];

    if (length >= 1264)
    {
        return -6;
    }
    m_seqNum++;
    bdDTLSData dataPacket(m_peerTag, m_seqNum.getValue());
    if (!dataPacket.serialize(outData, sizeof(outData), 0, &packetLength, &m_seqNum, m_sharedKey, reinterpret_cast<const bdUByte8* const>(data), length, &m_cypher, secID))
    {
        bdLogWarn("bdSocket/dtls", "Packet creation failed.");
        return -1;
    }
    bdAssert(packetLength < BD_MAX_DATAGRAM_SIZE, "overflow");
    return m_socket->sendTo(const_cast<bdAddr*>(addr), outData, packetLength);
}

const bdInt bdDTLSAssociation::getStatus() const
{
    bdNChar8 commonAddrInfo[1024];

    switch (m_state)
    {
    case BD_DTLS_CLOSED:
        return 3;
    case BD_DTLS_COOKIE_WAIT:
        return 3;
    case BD_DTLS_COOKIE_ECHOED:
        return 1;
    case BD_DTLS_ESTABLISHED:
        return 2;
    default:
        bdLogError("bdSocket/dtls", "Bad state!");
        bdCommonAddrInfo::getBriefInfo(m_addrHandle->m_endpoint.getCommonAddr(), commonAddrInfo, sizeof(commonAddrInfo));
        bdLogInfo("bdSocket/dtls", commonAddrInfo);
        return 3;
    }
}

const bdAddrHandleRef bdDTLSAssociation::getAddrHandle() const
{
    return bdAddrHandleRef(&m_addrHandle);
}

const bdSecurityID* bdDTLSAssociation::getLocalSecurityId() const
{
    return &m_localId;
}
