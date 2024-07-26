// Unit Include
#include "px_access_read.h"

// Qx Includes
#include <qx/core/qx-bitarray.h>

namespace PxCrypt
{
/*! @cond */

//===============================================================================================================
// PxAccessRead
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
PxAccessRead::PxAccessRead(const QImage* image, QByteArrayView seed) :
    mPixels(reinterpret_cast<const QRgb*>(image->bits())),
    mPxSequence(image->size(), seed),
    mChSequence(seed),
    mCurrentIndex(mPxSequence.next()),
    mCurrentChannel(mChSequence.next()),
    mAtEnd(false)
{
    //Q_ASSERT(image); // Will already crash above due to size() access
    Q_ASSERT(!seed.isEmpty());

    readMetaPixels();
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
/* These functions assume that the image has at least two pixels, which is safe
 * since the library should never create an instance of this class when the
 * size of the image is anywhere near that low because of its size checks
 */
bool PxAccessRead::advance()
{
    // Check for overall end
    if(!hasNextPixel())
    {
        mAtEnd = true;
        return false;
    }

    mCurrentIndex = mPxSequence.next();
    mCurrentChannel = mChSequence.next();

    return true;
}

quint8 PxAccessRead::readMetaValue()
{
    Qx::BitArray metaBits(3);

    // Read at 1 BPC
    for(uint i = 0; i < 3; i++)
    {
        metaBits.setBit(i, channelValue() & 0x01);
        nextChannel();
    }

    return metaBits.toInteger<quint8>();
}

void PxAccessRead::readMetaPixels()
{
    // Read BPC from first pixel
    mBpc = readMetaValue();

    // Read strat from second pixel
    mStrat = static_cast<EncStrat>(readMetaValue());

    // Should be at 3rd pixel
    Q_ASSERT(mPxSequence.pixelCoverage() == 3);
}

//Public:
quint8 PxAccessRead::bpc() const { return mBpc; }
EncStrat PxAccessRead::strat() const { return mStrat; }

bool PxAccessRead::hasNextPixel() const { return mPxSequence.hasNext(); }
bool PxAccessRead::pixelExhausted() const { return mChSequence.pixelExhausted(); }
bool PxAccessRead::atEnd() const { return mAtEnd; }

bool PxAccessRead::nextPixel()
{
    if(atEnd())
        return false;

    // Skip channel sequence its to end
    mChSequence.exhaust();

    return advance();
}

bool PxAccessRead::nextChannel()
{
    if(atEnd())
        return false;

    if(mChSequence.pixelExhausted())
        return advance();
    else
    {
        mCurrentChannel = mChSequence.next();
        return true;
    }
}

const QRgb& PxAccessRead::pixel() const { return mPixels[mCurrentIndex]; }
quint64 PxAccessRead::index() const { return mCurrentIndex; }
Channel PxAccessRead::channel() const { return mCurrentChannel; }

quint8 PxAccessRead::channelValue() const
{
    switch(mCurrentChannel)
    {
        case Channel::Red:
            return red();
        case Channel::Green:
            return green();
        case Channel::Blue:
            return blue();

        default:
            qCritical("Illegal channel in rotation");
    }

    return 0; // Never reached
}

quint8 PxAccessRead::red() const { return qRed(pixel()); }
quint8 PxAccessRead::green() const { return qGreen(pixel()); }
quint8 PxAccessRead::blue() const { return qBlue(pixel()); }
quint8 PxAccessRead::alpha() const { return qAlpha(pixel()); }

/*! @endcond */
}
