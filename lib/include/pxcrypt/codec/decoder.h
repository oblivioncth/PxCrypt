#ifndef DECODER_H
#define DECODER_H

// Shared Library Support
#include "pxcrypt/pxcrypt_codec_export.h"

// Qt Includes
#include <QByteArray>
#include <QScopedPointer>

namespace PxCrypt
{

class DecoderPrivate;

class PXCRYPT_CODEC_EXPORT Decoder
{
    Q_DECLARE_PRIVATE(Decoder);
//-Instance Variables----------------------------------------------------------------------------------------------
/*! @cond */
protected:
    std::unique_ptr<DecoderPrivate> d_ptr;
/*! @endcond */

//-Constructor---------------------------------------------------------------------------------------------------
/*! @cond */
protected:
    Decoder(std::unique_ptr<DecoderPrivate> d);
/*! @endcond */

//-Destructor---------------------------------------------------------------------------------------------------
public:
    virtual ~Decoder();

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    QByteArray presharedKey() const;
    void setPresharedKey(const QByteArray& key);
};

}

#endif // DECODER_H
