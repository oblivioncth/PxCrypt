#ifndef PX_WEAVER
#define PX_WEAVER

// Qt Includes
#include <QImage>

// Qx Includes

// Project Includes
#include "pxcrypt/encdec.h"
#include "px_sequence_generator.h"
#include "ch_sequence_generator.h"

namespace PxCrypt
{
/*! @cond */

class PxWeaver
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    EncType mType;
    QRgb* mPixels;
    PxSequenceGenerator mPxSequence;
    ChSequenceGenerator mChSequence;
    quint8 mClearMask;

    quint64 mPxIndex;
    quint8 mBuffer[4];
    bool mAtEnd;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    PxWeaver(QImage* canvas, QByteArrayView psk, quint8 bpc, EncType type);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void advance();

public:
    void weave(quint8 chunk);
    void flush();
};

/*! @endcond */
}

#endif // PX_WEAVER
