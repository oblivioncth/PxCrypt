#ifndef ENCODER_P_H
#define ENCODER_P_H

// Qt Includes
#include <QByteArray>

// Project Includes
#include "pxcrypt/codec/encoder.h"

namespace PxCrypt
{
/*! @cond */

class EncoderPrivate
{
//-Instance Variables----------------------------------------------------------------------------------------------
public:
    quint8 mBpc;
    Encoder::Encoding mEncoding;
    QByteArray mPsk;

//-Constructor---------------------------------------------------------------------------------------------------
protected:
    EncoderPrivate();

//-Destructor---------------------------------------------------------------------------------------------------
public:
    virtual ~EncoderPrivate();
};

/*! @endcond */
}

#endif // ENCODER_P_H
