// Unit Include
#include "standard.h"

// Qt Includes
#include <QDataStream>

// Qx Includes
#include <qx/core/qx-integrity.h>

namespace PxCryptPrivate
{

//===============================================================================================================
// StandardWork
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Protected:
StandardWork::StandardWork() :
    mChecksum(0)
{}

StandardWork::StandardWork(const QByteArray& tag, const QByteArray& payload) :
    mTag(tag),
    mChecksum(Qx::Integrity::crc32(payload)),
    mPayload(payload)
{
    if(mTag.size() > std::numeric_limits<tag_length_t>::max())
        mTag.resize(std::numeric_limits<tag_length_t>::max());
}

//-Class Functions----------------------------------------------------------------------------------------------
//Private:
quint64 StandardWork::renditionSize(tag_length_t tagSize, payload_length_t payloadSize)
{
    return sizeof(tag_length_t) +
           tagSize +
           sizeof(checksum_t) +
           sizeof(payload_length_t) +
           payloadSize;
}

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
quint64 StandardWork::renditionSize() const { return renditionSize(mTag.size(), mPayload.size()); }

ArtworkError StandardWork::renditionRead(QDataStream& stream)
{
    tag_length_t tl; stream >> tl;
    mTag.resize(tl);
    stream.readRawData(mTag.data(), tl);

    stream >> mChecksum;

    payload_length_t pl; stream >> pl;
    mPayload.resize(pl);
    stream.readRawData(mPayload.data(), pl);

    checksum_t sumCheck = Qx::Integrity::crc32(mPayload);
    if(mChecksum != sumCheck)
        return ArtworkError(ArtworkError::IntegrityError, u"The payload's checksum did not match its record."_s);

    return ArtworkError();
}

ArtworkError StandardWork::renditionWrite(QDataStream& stream) const
{
    stream << static_cast<tag_length_t>(mTag.size());
    stream.writeRawData(mTag.constData(), mTag.size());
    stream << mChecksum
           << static_cast<payload_length_t>(mPayload.size());
    stream.writeRawData(mPayload.constData(), mPayload.size());

    return ArtworkError();
}

//Public:
StandardWork::checksum_t StandardWork::checksum() const { return mChecksum; }
QByteArray StandardWork::tag() const { return mTag; }
QByteArray StandardWork::payload() const { return mPayload; }


//===============================================================================================================
// StandardWork::Measure
//===============================================================================================================

//-Constructor-----------------------------------------------------------------------------------------------------
//Public:
StandardWork::Measure::Measure() :
    Measure(0,0)
{}

StandardWork::Measure::Measure(tag_length_t tagSize, payload_length_t payloadSize) :
    mSize(StandardWork::renditionSize(tagSize, payloadSize))
{}


//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
quint64 StandardWork::Measure::renditionSize() const { return mSize; }

}
