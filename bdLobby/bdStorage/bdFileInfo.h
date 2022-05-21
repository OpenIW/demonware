// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdFileInfo : public bdTaskResult
{
public:
    enum bdVisibility : bdInt
    {
        BD_VISIBLE_PUBLIC = 0x0,
        BD_VISIBLE_PRIVATE = 0x1,
        BD_MAX_VISIBILITY_TYPE = 0x2,
    };
    bdUInt64 m_fileID;
    bdUInt m_createTime;
    bdUInt m_modifedTime;
    bdFileInfo::bdVisibility m_visibility;
    bdUInt64 m_ownerID;
    bdNChar8 m_fileName[128];
    bdUInt m_fileSize;

    void operator delete(void* p);
    void* operator new(bdUWord nbytes);
    bdFileInfo();
    ~bdFileInfo();
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
};
