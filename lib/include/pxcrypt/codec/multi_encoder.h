#ifndef MULTI_ENCODER_H
#define MULTI_ENCODER_H

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

class MultiEncoderPrivate;

class PXCRYPT_CODEC_EXPORT MultiEncoder final : public Encoder
{
    Q_DECLARE_PRIVATE(MultiEncoder);
//-Inner Classes----------------------------------------------------------------------------------------------
public:
    class Error;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    MultiEncoder();

//-Class Functions----------------------------------------------------------------------------------------------
public:
    static quint64 calculateMaximumPayload(const QList<QSize>& dims, quint16 tagSize, quint8 bpc);
    static quint8 calculateOptimalDensity(const QList<QSize>& dims, quint16 tagSize, quint32 payloadSize);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    QString tag() const;

    void setTag(const QByteArray& tag);

    Error encode(QList<QImage>& encoded, QByteArrayView payload, const QList<QImage>& mediums);
};

class PXCRYPT_CODEC_EXPORT QX_ERROR_TYPE(MultiEncoder::Error, "PxCrypt::MultiEncoder::Error", 6979)
{
//-Class Enums-------------------------------------------------------------
public:
    enum Type
    {
        NoError,
        MissingPayload,
        MissingMediums,
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
        {MissingMediums, u"No mediums were provided."_s},
        {InvalidImage, u"A medium is invalid."_s},
        {WontFit, u"A medium's dimensions are not large enough to fit the payload."_s},
        {InvalidBpc, u"Bits-per-channel must be between 1 and 7."_s},
        {WeaveFailed, u"There was an error while weaving data."_s}
    };
    //FIX ME: Make correct

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

#endif // MULTI_ENCODER_H
