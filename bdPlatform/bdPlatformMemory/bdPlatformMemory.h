// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

void* bdMemset(void* const s, const bdInt c, const bdUWord len);
void* bdMemcpy(void* const dest, const void* const src, const bdUWord len);
bdInt bdMemcmp(const void* const s1, const void* const s2, const bdUWord len);
void* bdMemmove(void* const dest, const void* const src, const bdUWord len);
