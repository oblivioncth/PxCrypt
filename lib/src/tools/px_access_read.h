#ifndef PX_ACCESS_READ_H
#define PX_ACCESS_READ_H

// Qt Includes
#include <QImage>

// Project Includes
#include "../encdec.h"
#include "ch_sequence_generator.h"
#include "px_sequence_generator.h"

namespace PxCrypt
{
/*! @cond */

class PxAccessRead
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    const QRgb* mPixels;
    quint8 mBpc;
    EncStrat mStrat;
    PxSequenceGenerator mPxSequence;
    ChSequenceGenerator mChSequence;

    quint64 mCurrentIndex;
    Channel mCurrentChannel;

    bool mAtEnd;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    PxAccessRead(const QImage* image, QByteArrayView seed);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    bool advance();
    quint8 readMetaValue();
    void readMetaPixels();

public:
    quint8 bpc() const;
    EncStrat strat() const;

    bool hasNextPixel() const;
    bool pixelExhausted() const;
    bool atEnd() const;

    bool nextPixel();
    bool nextChannel();

    const QRgb& pixel() const;

    quint64 index() const;
    Channel channel() const;
    quint8 channelValue() const;
    quint8 red() const;
    quint8 green() const;
    quint8 blue() const;
    quint8 alpha() const;
};

/*! @endcond */
}

#endif // PX_ACCESS_READ_H
