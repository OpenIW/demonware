// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdConnection/bdConnection.h"

bdReceivedMessage::bdReceivedMessage(const bdMessageRef message, const bdConnectionRef connection) : m_message(message), m_connection(connection)
{
}

bdMessageRef bdReceivedMessage::getMessage() const
{
    return bdMessageRef(&m_message);
}

bdConnectionRef bdReceivedMessage::getConnection() const
{
    return bdConnectionRef(&m_connection);
}
