// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdReceivedMessage
{
protected:
    bdMessageRef m_message;
    bdConnectionRef m_connection;
public:
    bdReceivedMessage(const bdMessageRef message, const bdConnectionRef connection);
    bdMessageRef getMessage() const;
    bdConnectionRef getConnection() const;
};
