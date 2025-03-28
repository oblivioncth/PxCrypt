#ifndef DECODER_P_H
#define DECODER_P_H

// Qt Includes
#include <QByteArray>

namespace PxCrypt
{
/*! @cond */

class DecoderPrivate
{
//-Instance Variables----------------------------------------------------------------------------------------------
public:
    QByteArray mPsk;

//-Constructor---------------------------------------------------------------------------------------------------
protected:
    DecoderPrivate();

//-Destructor---------------------------------------------------------------------------------------------------
public:
    virtual ~DecoderPrivate();
};

/*! @endcond */
}

#endif // DECODER_P_H
