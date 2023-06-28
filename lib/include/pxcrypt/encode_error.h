#ifndef ENCODE_ERROR_H
#define ENCODE_ERROR_H

// Shared Library Support
#include "pxcrypt/pxcrypt_codec_export.h"

// Qx Includes
#include <qx/core/qx-abstracterror.h>

namespace PxCrypt
{

class PXCRYPT_CODEC_EXPORT QX_ERROR_TYPE(EncodeError, "PxCrypt::EncoderError", 6978)
{
    friend class Encoder;
//-Class Enums------------------------------------------------------------------------------------------------
public:
    enum Type
    {
        NoError = 0,
        MissingPayload = 1,
        InvalidImage = 2,
        WontFit = 3,
        InvalidBpc = 4
    };

//-Class Variables------------------------------------------------------------------------------------------------
private:
    static inline const QString PRIMARY_STRING = QSL("Encoding failed.");

//-Instance Variables------------------------------------------------------------------------------------------------
private:
    Type mType;
    QString mString;

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    EncodeError();

private:
    EncodeError(Type type, QString errStr);

//-Instance Functions---------------------------------------------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;

    template<typename... Args>
    EncodeError arged(Args... args) const
    {
        EncodeError a = *this;
        a.mString = a.mString.arg(args...);
        return a;
    }

public:
    bool isValid() const;
    Type type() const;
    QString errorString() const;
};


}

#endif // ENCODE_ERROR_H
