// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdTaskResultProcessor
{
public:
    virtual ~bdTaskResultProcessor();
    virtual bdBool processResult(bdTaskResult*, bdUInt);
};