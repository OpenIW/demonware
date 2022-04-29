// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdRunnable
{
public:
    bdBool m_stop;

    virtual ~bdRunnable();
    virtual void stop();
    virtual bdUInt run(void* args) { return 0; };
    bdRunnable();
    void start();
};