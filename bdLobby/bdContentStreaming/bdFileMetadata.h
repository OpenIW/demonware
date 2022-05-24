// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

class bdFileMetaData : public bdTaskResult
{
public:
    enum bdVisibility
    {
        BD_VISIBLE_PUBLIC = 0,
        BD_VISIBLE_PRIVATE = 1,
        BD_MAX_VISIBILITY_TYPE = 2
    };

    bdUInt64 m_fileID;
    bdUInt m_createTime;
    bdUInt m_modifedTime;
    bdUInt m_fileSize;
    bdUInt64 m_ownerID;
    bdNChar8 m_ownerName[64];
    bdUInt16 m_fileSlot;
    bdNChar8 m_fileName[128];
    bdNChar8 m_url[384];
    bdUInt16 m_category;
    bdUInt m_numTags;
    bdUByte8 m_metaData[512];
    bdUInt m_metaDataSize;
    bdUInt m_summaryFileSize;
    bdTag m_tags[40];

    void* operator new(const bdUWord nbytes);
    void operator delete(void* p);
    void* operator new(const bdUWord nbytes, void* p);
    bdFileMetaData();
    ~bdFileMetaData();
    void reset();
    void resetArrays();
    virtual bdBool deserialize(bdByteBufferRef buffer);
    virtual bdUInt sizeOf();
    bdFileMetaData* operator=(const bdFileMetaData* other);
};
