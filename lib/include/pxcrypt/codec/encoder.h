#ifndef ENCODER_H
#define ENCODER_H

// Shared Library Support
#include "pxcrypt/pxcrypt_codec_export.h"

// Qt Includes
#include <QByteArray>
#include <QScopedPointer>

namespace PxCrypt
{

class EncoderPrivate;

class PXCRYPT_CODEC_EXPORT Encoder
{
    Q_DECLARE_PRIVATE(Encoder);
//-Class Enums--------------------------------------------------------------------------------------------------
public:
    /* TODO: Actually...  consider putting back public api encdec.h so that this can be there (and maybe even
     * some of the stuff from the private version), so that including decoder.h doesnt have to include encoder.h
     */
    enum Encoding : quint8
    {
        Relative,
        Absolute
    };

//-Instance Variables----------------------------------------------------------------------------------------------
/*! @cond */
protected:
    std::unique_ptr<EncoderPrivate> d_ptr;
/*! @endcond */

//-Constructor---------------------------------------------------------------------------------------------------
protected:
/*! @cond */
    Encoder(std::unique_ptr<EncoderPrivate> d_ptr);
/*! @endcond */

//-Destructor---------------------------------------------------------------------------------------------------
public:
    virtual ~Encoder();

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    quint8 bpc() const;
    Encoding encoding() const;
    QByteArray presharedKey() const;

    void setBpc(quint8 bpc);
    void setEncoding(Encoding enc);
    void setPresharedKey(const QByteArray& key);
};

}

#endif // ENCODER_H
