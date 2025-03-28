#ifndef STANDARD_ENCODER_H
#define STANDARD_ENCODER_H

// Shared Library Support
#include "pxcrypt/pxcrypt_codec_export.h"

// Qt Includes
#include <QImage>

// Qx Includes
#include <qx/core/qx-abstracterror.h>

// Project Includes
#include "pxcrypt/codec/encoder.h"

namespace PxCrypt
{

class StandardEncoderPrivate;

class PXCRYPT_CODEC_EXPORT StandardEncoder final : public Encoder
{
    Q_DECLARE_PRIVATE(StandardEncoder);
//-Inner Classes----------------------------------------------------------------------------------------------
public:
    class Error;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    StandardEncoder();

//-Class Functions----------------------------------------------------------------------------------------------
public:
    static quint64 calculateMaximumPayload(const QSize& dim, quint16 tagSize, quint8 bpc);
    static quint8 calculateOptimalDensity(const QSize& dim, quint16 tagSize, quint32 payloadSize);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    QString tag() const;

    void setTag(const QByteArray& tag);

    Error encode(QImage& encoded, QByteArrayView payload, const QImage& medium);
};

class PXCRYPT_CODEC_EXPORT QX_ERROR_TYPE(StandardEncoder::Error, "PxCrypt::StandardEncoder::Error", 6978)
{
//-Class Enums-------------------------------------------------------------
public:
    enum Type
    {
        NoError,
        MissingPayload,
        InvalidImage,
        WontFit,
        InvalidBpc,
        WeaveFailed
    };

//-Class Variables-------------------------------------------------------------
private:
    static inline const QString PREFIX_STRING = u"Encoding failed."_s;
    static inline const QHash<Type, QString> ERR_STRINGS{
        {NoError, u""_s},
        {MissingPayload, u"No payload data was provided."_s},
        {InvalidImage, u"The medium is invalid."_s},
        {WontFit, u"The medium's dimensions are not large enough to fit the payload."_s},
        {InvalidBpc, u"Bits-per-channel must be between 1 and 7."_s},
        {WeaveFailed, u"There was an error while weaving data."_s}
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

#endif // STANDARD_ENCODER_H
