#ifndef STANDARD_H
#define STANDARD_H

// Qt Includes
#include <QByteArray>
#include <QString>

// Project Includes
#include "art_io/artwork.h"

namespace PxCryptPrivate
{

class StandardWork : public Artwork<StandardWork, 1>
{
//-Inner Class------------------------------------------------------------------------------------------------------------
public:
    class Measure;

//-Class Types------------------------------------------------------------------------------------------------------------
public:
    using checksum_t = quint32;
    using tag_length_t = quint16;
    using payload_length_t = quint32;

//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QByteArray mTag;
    checksum_t mChecksum;
    QByteArray mPayload;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    StandardWork();
    StandardWork(const QByteArray& tag, const QByteArray& payload);

//-Class Functions----------------------------------------------------------------------------------------------
private:
    static quint64 renditionSize(tag_length_t tagSize, payload_length_t payloadSize);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    quint64 renditionSize() const override;
    ArtworkError renditionRead(QDataStream& stream) override;
    ArtworkError renditionWrite(QDataStream& stream) const override;

public:
    checksum_t checksum() const;
    QByteArray tag() const;
    QByteArray payload() const;
};

class StandardWork::Measure : public IMeasure
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

#endif // STANDARD_H
