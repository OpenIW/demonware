// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

void* bdAlignedOffsetMalloc(const bdUWord size, const bdUWord align, const bdUWord offset);
void bdAlignedOffsetFree(void* p);
void* bdAlignedOffsetRealloc(void* p, const bdUWord origSize, const bdUWord size, const bdUWord align, const bdUWord offset);
