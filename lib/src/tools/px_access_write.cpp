// Unit Include
#include "px_access_write.h"

// Qx Includes
#include <qx/core/qx-bitarray.h>

// Magic Enum Include
#include <magic_enum.hpp>

namespace PxCrypt
{
/*! @cond */

//===============================================================================================================
// PxAccessWrite
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
PxAccessWrite::PxAccessWrite(QImage* image, QByteArrayView seed, quint8 bpc, EncType type) :
    mPixels(reinterpret_cast<QRgb*>(image->bits())),
    mBpc(bpc),
    mType(type),
    mPxSequence(image->size(), seed),
    mChSequence(seed),
    mCurrentIndex(mPxSequence.next()),
    mCurrentChannel(mChSequence.next()),
    mAtEnd(false)
{
    Q_ASSERT(mBpc > 0 && mBpc < 8);

    writeMetaPixels();
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
/* NOTE: These functions assume that the image has at least two pixels, which is safe
 * since the library should never create an instance of this class when the
 * size of the image is anywhere near that low because of its size checks
 */
bool PxAccessWrite::advance()
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

void PxAccessWrite::writeMetaValue(quint8 value)
{
    // Write buffer
    static quint8 bBuff[3];

    // Get bits
    Qx::BitArray metaBits = Qx::BitArray::fromInteger(value);

    // Write at 1 BPC
    for(uint i = 0; i < 3; i++)
    {
        bBuff[channel()] = (channelValue() & 0xFFFE) | static_cast<uint>(metaBits.testBit(i));
        if(i == 2)
            pixel() = qRgba(bBuff[Channel::Red], bBuff[Channel::Green], bBuff[Channel::Blue], alpha());
        nextChannel();
    }
}

void PxAccessWrite::writeMetaPixels()
{
    /* This writes BPC and EncType to the canvas in sequence as normal, but with at
     * a temporary standard density of 1 bpp.
     *
     * This assumes that no EncType is > 7
     */
    //static_assert(std::max_element(magic_enum::enum_values<EncType>()) <= 7, "");
    static_assert([]{
       auto vals = magic_enum::enum_values<EncType>();
       return *std::max_element(vals.cbegin(), vals.cend()) <= 7;
    },
    "EncType must have no members with a value > 7 for this to work!");

    // Set BPC on first pixel
    writeMetaValue(mBpc);

    // Set encoding type on second pixel
    writeMetaValue(static_cast<quint8>(mType));

    // Should be at 3rd pixel
    Q_ASSERT(mPxSequence.pixelCoverage() == 3);
}

//Public:
quint8 PxAccessWrite::bpc() const { return mBpc; }
EncType PxAccessWrite::type() const { return mType; }

bool PxAccessWrite::hasNextPixel() const { return mPxSequence.hasNext(); }
bool PxAccessWrite::pixelExhausted() const { return mChSequence.pixelExhausted(); }
bool PxAccessWrite::atEnd() const { return mAtEnd; }

bool PxAccessWrite::nextPixel()
{
    if(atEnd())
        return false;

    // Skip channel sequence its to end
    mChSequence.exhaust();

    return advance();
}

bool PxAccessWrite::nextChannel()
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

QRgb& PxAccessWrite::pixel() { return mPixels[mCurrentIndex]; }
const QRgb& PxAccessWrite::pixelConst() const { return mPixels[mCurrentIndex]; }
quint64 PxAccessWrite::index() const { return mCurrentIndex; }
Channel PxAccessWrite::channel() const { return mCurrentChannel; }

quint8 PxAccessWrite::channelValue() const
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

quint8 PxAccessWrite::red() const { return qRed(pixelConst()); }
quint8 PxAccessWrite::green() const { return qGreen(pixelConst()); }
quint8 PxAccessWrite::blue() const { return qBlue(pixelConst()); }
quint8 PxAccessWrite::alpha() const { return qAlpha(pixelConst()); }

/*! @endcond */
}
