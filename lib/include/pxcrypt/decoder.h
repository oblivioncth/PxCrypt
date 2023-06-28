#ifndef DECODER_H
#define DECODER_H

// Shared Library Support
#include "pxcrypt/pxcrypt_codec_export.h"

// Qt Includes
#include <QImage>

// Qx Includes
#include <qx/core/qx-genericerror.h>
#include <qx/utility/qx-macros.h>

namespace PxCrypt
{

class PXCRYPT_CODEC_EXPORT Decoder
{
//-Class Variables----------------------------------------------------------------------------------------------
private:
    // Errors
    const QString ERR_DECODING_FAILED = QSL("Decoding failed.");
    const QString ERR_INVALID_SOURCE = QSL("The encoded image is invalid.");
    const QString ERR_MISSING_MEDIUM = QSL("The encoded image requires a medium to be decoded but none was provided.");
    const QString ERR_DIMMENSION_MISTMATCH = QSL("The required medium image has different dimensions than the encoded image.");
    const QString ERR_NOT_LARGE_ENOUGH = QSL("The provided image is not large enough to be an encoded image.");
    const QString ERR_INVALID_META = QSL("The provided image is not encoded.");
    const QString ERR_INVALID_HEADER = QSL("The provided image is not encoded or the password/medium are incorrect.");
    const QString ERR_LENGTH_MISMATCH = QSL("The encoded image's header indicates it contains more data than possible.");
    const QString ERR_UNEXPECTED_END = QSL("All pixels were skimmed before the expected payload size was reached.");
    const QString ERR_CHECKSUM_MISMATCH = QSL("The payload's checksum did not match the expected value.");

//-Instance Variables----------------------------------------------------------------------------------------------
private:
    // Settings
    QByteArray mPsk;

    // Data
    QString mTag;

    // Error Status
    Qx::GenericError mError;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    Decoder();

//-Class Functions------------------------------------------------------------------------------------------------
private:
    static bool canFitHeader(const QSize& dim, quint8 bpc);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    bool hasError() const;
    Qx::GenericError error() const;
    void reset();

    QString tag() const;
    QByteArray presharedKey() const;

    void setPresharedKey(const QByteArray& key);

    QByteArray decode(const QImage& encoded, const QImage& medium = QImage());
};

}

#endif // DECODER_H
