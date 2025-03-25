#ifndef MULTIPART_H
#define MULTIPART_H

// Qt Includes
#include <QByteArray>
#include <QString>

// Project Includes
#include "art_io/artwork.h"

namespace PxCryptPrivate
{

class MultiPartWork : public Artwork<MultiPartWork, 2>
{
//-Inner Class------------------------------------------------------------------------------------------------------------
public:
    class Measure;

//-Class Types------------------------------------------------------------------------------------------------------------
public:
    using checksum_t = quint32;
    using tag_length_t = quint16;
    using payload_length_t = quint32;
    using part_idx_t = quint16;

//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QByteArray mTag;
    checksum_t mPartChecksum;
    checksum_t mCompleteCheckum;
    part_idx_t mPartIdx;
    part_idx_t mPartCount;
    QByteArray mPartPayload;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    MultiPartWork();
    MultiPartWork(const QByteArray& tag, const QByteArray& partPayload, checksum_t completeChecksum, part_idx_t partIdx, part_idx_t partCount);

//-Class Functions----------------------------------------------------------------------------------------------
private:
    static quint64 renditionSize(tag_length_t tagSize, payload_length_t payloadSize);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    quint64 renditionSize() const override;
    ArtworkError renditionRead(QDataStream& stream) override;
    ArtworkError renditionWrite(QDataStream& stream) const override;

public:
    QByteArray tag() const;
    checksum_t partChecksum() const;
    checksum_t completeChecksum() const;
    part_idx_t partIdx() const;
    part_idx_t partCount() const;
    QByteArray partPayload() const;
};

class MultiPartWork::Measure : public IMeasure
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    quint64 mSize;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    Measure(); // Useful for measuring bare minimum consumption
    Measure(tag_length_t tagSize, payload_length_t payloadSize);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    quint64 renditionSize() const override;
};

}

#endif // MULTIPART_H
