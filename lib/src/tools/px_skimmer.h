#ifndef PX_SKIMMER
#define PX_SKIMMER

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

class PxSkimmer
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    EncType mType;
    const QRgb* mPixels;
    const QRgb* mRefPixels;

    PxSequenceGenerator mPxSequence;
    ChSequenceGenerator mChSequence;

    quint8 mMask;
    quint8 mBuffer[3];

    bool mLimitReached;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    PxSkimmer(const QImage* surface, const QImage* medium, QByteArrayView psk, quint8 bpc, EncType type);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void advance();

public:
    bool isAtLimit();
    quint8 next();
};

/*! @endcond */
}

#endif // PX_SKIMMER
