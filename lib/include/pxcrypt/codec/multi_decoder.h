#ifndef MULTI_DECODER_H
#define MULTI_DECODER_H

// Shared Library Support
#include "pxcrypt/pxcrypt_codec_export.h"

// Qt Includes
#include <QImage>

// Qx Includes
#include <qx/core/qx-abstracterror.h>

// Project Includes
#include "pxcrypt/codec/decoder.h"

namespace PxCrypt
{

class MultiDecoderPrivate;

class PXCRYPT_CODEC_EXPORT MultiDecoder final : public Decoder
{
    Q_DECLARE_PRIVATE(MultiDecoder);
//-Inner Classes----------------------------------------------------------------------------------------------
public:
    class Error;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    MultiDecoder();

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    QString tag() const;
    Error decode(QByteArray& decoded, const QList<QImage>& encoded, const QList<QImage>& mediums = {});
};

class PXCRYPT_CODEC_EXPORT QX_ERROR_TYPE(MultiDecoder::Error, "PxCrypt::MultiDecoder::Error", 6879)
{
//-Class Enums-------------------------------------------------------------
public:
    enum Type
    {
        NoError,
        MissingSources,
        InvalidSource,
        MissingMediums,
        DimensionMismatch,
        NotLargeEnough,
        InvalidMeta,
        PartMismatch,
        PartsMissing,
        ChecksumMismatch,
        SkimFailed
    };

//-Class Variables-------------------------------------------------------------
private:
    static inline const QString PREFIX_STRING = u"Decoding failed."_s;
    static inline const QHash<Type, QString> ERR_STRINGS{
        {NoError, u""_s},
        {MissingSources, u"No encoded images were provided."_s},
        {InvalidSource, u"The encoded image is invalid."_s},
        {MissingMediums, u"The encoded images require mediums to be decoded but none or not enough were provided, or some were invalid..."_s},
        {DimensionMismatch, u"One of the required medium image has different dimensions than the encoded image."_s},
        {NotLargeEnough, u"The provided image is not large enough to be an encoded image."_s},
        {InvalidMeta, u"The provided image is not encoded."_s},
        {PartMismatch, u"One of the parts had a different complete checksum or tag than the rest, or an unexpected part index."_s},
        {PartsMissing, u"The image set consists of more parts than were provided."_s},
        {ChecksumMismatch, u"The full payload checksum did not match that of the reassembled payload."_s},
        {SkimFailed, u"There was an error while skimming data."_s}
    };

//-Instance Variables-------------------------------------------------------------
private:
    Type mType;
    QString mSpecific;
    qsizetype mImageIndex;

//-Constructor-------------------------------------------------------------
public:
    Error(Type t = NoError, qsizetype idx = -1, const QString& s = {});

//-Instance Functions-------------------------------------------------------------
public:
    bool isValid() const;
    Type type() const;
    qsizetype imageIndex() const;
    QString specific() const;
    QString errorString() const;

private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
};

}

#endif // MULTI_DECODER_H
