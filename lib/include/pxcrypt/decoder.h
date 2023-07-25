#ifndef DECODER_H
#define DECODER_H

// Shared Library Support
#include "pxcrypt/pxcrypt_codec_export.h"

// Qt Includes
#include <QImage>

// Project Includes
#include "pxcrypt/decode_error.h"

namespace PxCrypt
{

class PXCRYPT_CODEC_EXPORT Decoder
{
//-Class Variables----------------------------------------------------------------------------------------------
private:
    // Errors
    static inline const DecodeError ERR_INVALID_SOURCE =
        DecodeError(DecodeError::InvalidSource, u"The encoded image is invalid."_s);
    static inline const DecodeError ERR_MISSING_MEDIUM =
        DecodeError(DecodeError::MissingMedium, u"The encoded image requires a medium to be decoded but none was provided."_s);
    static inline const DecodeError ERR_DIMENSION_MISTMATCH =
        DecodeError(DecodeError::DimensionMismatch, u"The required medium image has different dimensions than the encoded image."_s);
    static inline const DecodeError ERR_NOT_LARGE_ENOUGH =
        DecodeError(DecodeError::NotLargeEnough, u"The provided image is not large enough to be an encoded image."_s);
    static inline const DecodeError ERR_INVALID_META =
        DecodeError(DecodeError::InvalidMeta, u"The provided image is not encoded."_s);
    static inline const DecodeError ERR_INVALID_HEADER =
        DecodeError(DecodeError::InvalidHeader, u"The provided image is not encoded or the password/medium are incorrect."_s);
    static inline const DecodeError ERR_LENGTH_MISMATCH =
        DecodeError(DecodeError::LengthMismatch, u"The encoded image's header indicates it contains more data than possible."_s);
    static inline const DecodeError ERR_UNEXPECTED_END =
        DecodeError(DecodeError::UnexpectedEnd, u"All pixels were skimmed before the expected payload size was reached."_s);
    static inline const DecodeError ERR_CHECKSUM_MISMATCH =
        DecodeError(DecodeError::ChecksumMismatch, u"The payload's checksum did not match the expected value."_s);

//-Instance Variables----------------------------------------------------------------------------------------------
private:
    // Settings
    QByteArray mPsk;

    // Data
    QString mTag;

    // Error Status
    DecodeError mError;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    Decoder();

//-Class Functions------------------------------------------------------------------------------------------------
private:
    static bool canFitHeader(const QSize& dim, quint8 bpc);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    bool hasError() const;
    DecodeError error() const;
    void reset();

    QString tag() const;
    QByteArray presharedKey() const;

    void setPresharedKey(const QByteArray& key);

    QByteArray decode(const QImage& encoded, const QImage& medium = QImage());
};

}

#endif // DECODER_H
