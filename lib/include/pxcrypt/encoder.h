#ifndef ENCODER_H
#define ENCODER_H

// Shared Library Support
#include "pxcrypt/pxcrypt_codec_export.h"

// Qt Includes
#include <QImage>

// Qx Includes
#include <qx/utility/qx-macros.h>

// Project Includes
#include "pxcrypt/encode_error.h"

namespace PxCrypt
{

class PXCRYPT_CODEC_EXPORT Encoder
{
//-Class Enums--------------------------------------------------------------------------------------------------
public:
    enum Encoding : quint8
    {
        Relative,
        Absolute
    };

//-Class Variables----------------------------------------------------------------------------------------------
private:
    // Errors
    static inline const EncodeError ERR_MISSING_PAYLOAD =
            EncodeError(EncodeError::MissingPayload, QSL("No payload data was provided."));
    static inline const EncodeError ERR_INVALID_IMAGE =
            EncodeError(EncodeError::InvalidImage, QSL("The medium is invalid."));
    static inline const EncodeError ERR_WONT_FIT =
            EncodeError(EncodeError::WontFit, QSL("The medium's dimensions are not large enough to fit the payload (%1 KiB short)."));
    static inline const EncodeError ERR_INVALID_BPC =
            EncodeError(EncodeError::InvalidBpc, QSL("Bits-per-channel must be between 1 and 7."));

//-Instance Variables----------------------------------------------------------------------------------------------
private:
    // Settings
    quint8 mBpc;
    QByteArray mPsk;
    Encoding mEncoding;

    // Data
    QString mTag;

    // Error Status
    EncodeError mError;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    Encoder();

//-Class Functions----------------------------------------------------------------------------------------------
public:
    static quint64 calculateMaximumStorage(const QSize& dim, quint16 tagSize, quint8 bpc);
    static quint8 calculateOptimalDensity(const QSize& dim, quint16 tagSize, quint32 payloadSize);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    bool hasError() const;
    EncodeError error() const;
    void reset();

    quint8 bpc() const;
    QByteArray presharedKey() const;
    Encoding encoding() const;
    QString tag() const;

    void setBpc(quint8 bpc);
    void setPresharedKey(const QByteArray& key);
    void setEncoding(Encoding enc);
    void setTag(const QString tag);

    QImage encode(QByteArrayView payload, const QImage& medium);
};

}

#endif // ENCODER_H
