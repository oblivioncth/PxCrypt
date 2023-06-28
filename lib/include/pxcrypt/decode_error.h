#ifndef DECODE_ERROR_H
#define DECODE_ERROR_H

// Shared Library Support
#include "pxcrypt/pxcrypt_codec_export.h"

// Qx Includes
#include <qx/core/qx-abstracterror.h>

namespace PxCrypt
{

class PXCRYPT_CODEC_EXPORT QX_ERROR_TYPE(DecodeError, "PxCrypt::DecoderError", 6878)
{
    friend class Decoder;
//-Class Enums------------------------------------------------------------------------------------------------
public:
    enum Type
    {
        NoError = 0,
        InvalidSource = 1,
        MissingMedium = 2,
        DimensionMismatch = 3,
        NotLargeEnough = 4,
        InvalidMeta = 5,
        InvalidHeader = 6,
        LengthMismatch = 7,
        UnexpectedEnd = 8,
        ChecksumMismatch = 9
    };

//-Class Variables------------------------------------------------------------------------------------------------
private:
    static inline const QString PRIMARY_STRING = QSL("Decoding failed.");

//-Instance Variables------------------------------------------------------------------------------------------------
private:
    Type mType;
    QString mString;

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    DecodeError();

private:
    DecodeError(Type type, QString errStr);

//-Instance Functions---------------------------------------------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;

    template<typename... Args>
    DecodeError arged(Args... args) const
    {
        DecodeError a = *this;
        a.mString = a.mString.arg(&args...);
    }

public:
    bool isValid() const;
    Type type() const;
    QString errorString() const;
};

}

#endif // DECODE_ERROR_H
