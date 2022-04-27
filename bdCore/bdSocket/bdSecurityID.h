// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdSecurityID
{
public:
	char ab[8];

	bdSecurityID();
	bdSecurityID(const bdSecurityID* other);
	bdBool operator==(const bdSecurityID* other);
	bdSecurityID* operator=(const bdSecurityID* other);
	bdBool operator!=(const bdSecurityID* other);
};