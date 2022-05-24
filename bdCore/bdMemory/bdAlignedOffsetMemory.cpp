// SPDX-License-Identifier: GPL-3.0-or-later

#include "bdCore/bdCore.h"

void* bdAlignedOffsetMalloc(const bdUWord size, const bdUWord align, const bdUWord offset)
{
    void* dataPtr;

    if ((align & (align - 1)) != 0)
    {
        return 0;
    }
    bdUWord padding = align + offset + 4;
    dataPtr = malloc(size + padding);
    if (!dataPtr)
    {
        return 0;
    }
    *(int*)((~(align - 1) & ((unsigned int)dataPtr + padding)) - offset - 4) = (int)dataPtr;
    return (void*)((~(align - 1) & ((unsigned int)dataPtr + padding)) - offset);
}

void bdAlignedOffsetFree(void* p)
{
    free(*((void**)p - 1));
}

void* bdAlignedOffsetRealloc(void* p, const bdUWord origSize, const bdUWord size, const bdUWord align, const bdUWord offset)
{
    void* dataPtr;

    dataPtr = bdAlignedOffsetMalloc(size, align, offset);
    bdMemcpy(dataPtr, p, size >= origSize ? origSize : size);
    bdAlignedOffsetFree(p);
    return dataPtr;
}
