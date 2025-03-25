// Unit Include
#include "multipart.h"

// Qt Includes
#include <QDataStream>

// Qx Includes
#include <qx/core/qx-integrity.h>

namespace PxCryptPrivate
{

//===============================================================================================================
// MultiPartWork
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Protected:
MultiPartWork::MultiPartWork() :
    mPartChecksum(0),
    mCompleteCheckum(0),
    mPartIdx(0),
    mPartCount(0)
{}

MultiPartWork::MultiPartWork(const QByteArray& tag, const QByteArray& payload, checksum_t completeChecksum, part_idx_t partIdx, part_idx_t partCount) :
    mTag(tag),
    mPartChecksum(Qx::Integrity::crc32(payload)),
    mCompleteCheckum(completeChecksum),
    mPartIdx(partIdx),
    mPartCount(partCount),
    mPartPayload(payload)
{
    if(mTag.size() > std::numeric_limits<tag_length_t>::max())
        mTag.resize(std::numeric_limits<tag_length_t>::max());
}

//-Class Functions----------------------------------------------------------------------------------------------
//Private:
quint64 MultiPartWork::renditionSize(tag_length_t tagSize, payload_length_t payloadSize)
{
    return sizeof(tag_length_t) +
           tagSize +
           sizeof(checksum_t) + // Part Checksum
           sizeof(checksum_t) + // Complete Checksum
           sizeof(part_idx_t) + // Part Idx
           sizeof(part_idx_t) + // Part Count
           sizeof(payload_length_t) +
           payloadSize;
}

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
quint64 MultiPartWork::renditionSize() const { return renditionSize(mTag.size(), mPartPayload.size()); }

ArtworkError MultiPartWork::renditionRead(QDataStream& stream)
{
    // Read Tag
    tag_length_t tl; stream >> tl;
    mTag.resize(tl);
    stream.readRawData(mTag.data(), tl);

    // Read Checksums
    stream >> mPartChecksum >> mCompleteCheckum;

    // Read Part Indices
    stream >> mPartIdx >> mPartCount;

    // Read payload
    payload_length_t pl; stream >> pl;
    mPartPayload.resize(pl);
    stream.readRawData(mPartPayload.data(), pl);

    // Confirm checksum
    if(auto sumCheck = Qx::Integrity::crc32(mPartPayload); sumCheck != mPartChecksum)
        return ArtworkError(ArtworkError::IntegrityError, u"The payload's checksum did not match its record."_s);

    return ArtworkError();
}

ArtworkError MultiPartWork::renditionWrite(QDataStream& stream) const
{
    // Write Tag
    stream << static_cast<tag_length_t>(mTag.size());
    stream.writeRawData(mTag.constData(), mTag.size());

    // Write Checksums
    stream << mPartChecksum << mCompleteCheckum;

    // Write Part Indices
    stream << mPartIdx << mPartCount;

    // Write Payload
    stream << static_cast<payload_length_t>(mPartPayload.size());
    stream.writeRawData(mPartPayload.constData(), mPartPayload.size());

    return ArtworkError();
}

//Public:
QByteArray MultiPartWork::tag() const { return mTag; }
MultiPartWork::checksum_t MultiPartWork::partChecksum() const { return mPartChecksum; }
MultiPartWork::checksum_t MultiPartWork::completeChecksum() const { return mCompleteCheckum; }
MultiPartWork::part_idx_t MultiPartWork::partIdx() const { return mPartIdx; }
MultiPartWork::part_idx_t MultiPartWork::partCount() const { return mPartCount; }
QByteArray MultiPartWork::partPayload() const { return mPartPayload; }


//===============================================================================================================
// MultiPartWork::Measure
//===============================================================================================================

//-Constructor-----------------------------------------------------------------------------------------------------
//Public:
MultiPartWork::Measure::Measure() :
    Measure(0,0)
{}

MultiPartWork::Measure::Measure(tag_length_t tagSize, payload_length_t payloadSize) :
    mSize(MultiPartWork::renditionSize(tagSize, payloadSize))
{}


//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
quint64 MultiPartWork::Measure::renditionSize() const { return mSize; }

}
