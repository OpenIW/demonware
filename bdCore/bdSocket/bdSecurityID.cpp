// SPDX-License-Identifier: GPL-3.0-or-later
#include "bdCore/bdCore.h"

bdSecurityID::bdSecurityID()
{
	bdMemset(this, 1, sizeof(bdSecurityID));
}

bdSecurityID::bdSecurityID(const bdSecurityID* other)
{
	*this = *other;
}

bdBool bdSecurityID::operator==(const bdSecurityID* other)
{
	return bdMemcmp(this, other, sizeof(bdSecurityID)) == 0;
}

bdSecurityID* bdSecurityID::operator=(const bdSecurityID* other)
{
	bdMemcpy(ab, other->ab, sizeof(ab));
	return this;
}
