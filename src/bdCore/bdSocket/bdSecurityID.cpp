// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

bdSecurityID::bdSecurityID()
{
    bdMemset(ab, 1, sizeof(ab));
}

bdSecurityID::bdSecurityID(const bdSecurityID& other)
{
   bdMemcpy(ab, other.ab, sizeof(ab));
}

bdBool bdSecurityID::operator==(const bdSecurityID& other) const
{
    return bdMemcmp(ab, other.ab, sizeof(bdSecurityID)) == 0;
}

bdBool bdSecurityID::operator!=(const bdSecurityID& other) const
{
    return !(this->ab == other.ab);
}
