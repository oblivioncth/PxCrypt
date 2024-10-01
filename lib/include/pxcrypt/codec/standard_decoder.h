#ifndef STANDARD_DECODER_H
#define STANDARD_DECODER_H

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

class StandardDecoderPrivate;

class PXCRYPT_CODEC_EXPORT StandardDecoder final : public Decoder
{
    Q_DECLARE_PRIVATE(StandardDecoder);
//-Inner Classes----------------------------------------------------------------------------------------------
public:
    class Error;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    StandardDecoder();

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    QString tag() const;
    Error decode(QByteArray& decoded, const QImage& encoded, const QImage& medium = QImage());
};

class PXCRYPT_CODEC_EXPORT QX_ERROR_TYPE(StandardDecoder::Error, "PxCrypt::StandardDecoder::Error", 6878)
{
//-Class Enums-------------------------------------------------------------
public:
    enum Type
    {
        NoError,
        InvalidSource,
        MissingMedium,
        DimensionMismatch,
        NotLargeEnough,
        InvalidMeta,
        InvalidHeader,
        LengthMismatch,
        UnexpectedEnd,
        ChecksumMismatch,
        SkimFailed
    };

//-Class Variables-------------------------------------------------------------
private:
    static inline const QString PREFIX_STRING = u"Decoding failed."_s;
    static inline const QHash<Type, QString> ERR_STRINGS{
        {NoError, u""_s},
        {InvalidSource, u"The encoded image is invalid."_s},
        {MissingMedium, u"The encoded image requires a medium to be decoded but none was provided."_s},
        {DimensionMismatch, u"The required medium image has different dimensions than the encoded image."_s},
        {NotLargeEnough, u"The provided image is not large enough to be an encoded image."_s},
        {InvalidMeta, u"The provided image is not encoded."_s},
        {SkimFailed, u"There was an error while skimming data."_s}
    };

//-Instance Variables-------------------------------------------------------------
private:
    Type mType;
    QString mSpecific;

//-Constructor-------------------------------------------------------------
public:
    Error(Type t = NoError, const QString& s = {});

//-Instance Functions-------------------------------------------------------------
public:
    bool isValid() const;
    Type type() const;
    QString specific() const;
    QString errorString() const;

private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
};

}

#endif // STANDARD_DECODER_H
