// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

bdSecurityKey::bdSecurityKey()
{
    bdMemset(ab, 1, sizeof(bdSecurityKey));
}

bdSecurityKey::bdSecurityKey(const bdSecurityKey& other)
{
    bdMemcpy(ab, other.ab, sizeof(bdSecurityKey));
}

bdBool bdSecurityKey::operator==(const bdSecurityKey& other)
{
    return bdMemcmp(ab, other.ab, sizeof(bdSecurityKey)) == 0;
}
