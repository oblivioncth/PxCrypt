#ifndef PX_WEAVER
#define PX_WEAVER

// Qt Includes
#include <QImage>

// Qx Includes

// Project Includes
#include "pxcrypt/encdec.h"
#include "px_sequence_generator.h"

namespace PxCrypt
{

class PxWeaver
{
//-Class Variables------------------------------------------------------------------------------------------------------
private:


//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    EncType mType;
    QRgb* mPixels;
    PxSequenceGenerator mSequence;
    quint8 mClearMask;

    quint64 mPxIndex;
    int mChannel;
    quint8 mBuffer[4];
    bool mAtEnd;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    PxWeaver(QImage* canvas, QStringView psk, quint8 bpc, EncType type);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void advance();

public:
    void weave(quint8 chunk);
    void flush();
};

}

#endif // PX_WEAVER
