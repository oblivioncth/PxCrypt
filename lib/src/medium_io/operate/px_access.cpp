// Unit Include
#include "px_access.h"

namespace PxCryptPrivate
{

//===============================================================================================================
// PxAccess
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
PxAccess::PxAccess(QImage* canvas, const FrameTraverser& frameTraverser, quint8 bpc, const QImage* refCanvas) :
    mPixels(reinterpret_cast<QRgb*>(canvas->bits())),
    mRefPixels(refCanvas ? reinterpret_cast<const QRgb*>(refCanvas->bits()) : nullptr),
    mTraverser(frameTraverser, bpc),
    mInitTravState(mTraverser.state()),
    mNeedFlush(false)
{
    Q_ASSERT(canvas && !canvas->isNull());
    reset();
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
QRgb& PxAccess::canvasPixel() { return mPixels[mTraverser.pixelIndex()]; }
const QRgb& PxAccess::constCanvasPixel() const { return mPixels[mTraverser.pixelIndex()]; }
quint8 PxAccess::canvasRed() const { return qRed(constCanvasPixel()); }
quint8 PxAccess::canvasGreen() const { return qGreen(constCanvasPixel()); }
quint8 PxAccess::canvasBlue() const { return qBlue(constCanvasPixel()); }
quint8 PxAccess::canvasAlpha() const { return qAlpha(constCanvasPixel()); }

const QRgb& PxAccess::referencePixel() const{ Q_ASSERT(mRefPixels); return mRefPixels[mTraverser.pixelIndex()]; }
quint8 PxAccess::referenceRed() const { return qRed(referencePixel()); }
quint8 PxAccess::referenceGreen() const { return qGreen(referencePixel()); }
quint8 PxAccess::referenceBlue() const { return qBlue(referencePixel()); }

void PxAccess::fillBuffer()
{
    mBuffer[Channel::Red] = canvasRed();
    mBuffer[Channel::Green] = canvasGreen();
    mBuffer[Channel::Blue] = canvasBlue();
    mBuffer[Channel::Alpha] = canvasAlpha();
}

void PxAccess::flushBuffer()
{
    if(!mNeedFlush)
        return;

    Q_ASSERT(mTraverser.pixelIndex() >= 0); // Can flush when not at frame end, but not when off frame

    canvasPixel() = qRgba(mBuffer[Channel::Red],
                          mBuffer[Channel::Green],
                          mBuffer[Channel::Blue],
                          mBuffer[Channel::Alpha]);
    mNeedFlush = false;
}

//Public:
quint8 PxAccess::bpc() const { return mTraverser.bpc(); }
int PxAccess::bitIndex() const { return mTraverser.channelBitIndex(); }
bool PxAccess::hasReferenceCanvas() const { return mRefPixels; }
bool PxAccess::atEnd() const { return mTraverser.atEnd(); }

void PxAccess::reset()
{
    // Ensure canvas is current
    flushBuffer();

    // Restart generator sequence to start (just before first non-meta-pixel)
    if(mTraverser != mInitTravState)
        mTraverser = CanvasTraverser(mInitTravState);

    // Ensure callbacks are set
    mTraverser.setPrePixelChange([this]{ flushBuffer(); });
    mTraverser.setPostPixelChange([this]{ fillBuffer(); });

    // Fill
    fillBuffer();
}

qint64 PxAccess::skip(qint64 bytes)
{
    // Ensure canvas is current
    flushBuffer();

    // Skip
    qint64 skipped = mTraverser.skip(bytes);

    // Fill if not at end
    if(!atEnd())
        fillBuffer();

    return skipped;
}

void PxAccess::advanceBits(int bitCount) { mTraverser.advanceBits(bitCount); }
void PxAccess::flush() { flushBuffer(); }

quint8& PxAccess::bufferedValue()
{
    mNeedFlush = true;
    return mBuffer[mTraverser.channel()];
}

quint8 PxAccess::constBufferedValue() const { return mBuffer[mTraverser.channel()]; }

quint8 PxAccess::originalValue() const
{
    switch(mTraverser.channel())
    {
    case Channel::Red:
        return canvasRed();
    case Channel::Green:
        return canvasGreen();
    case Channel::Blue:
        return canvasBlue();

    default:
        qCritical("Illegal channel in rotation");
    }

    return 0; // Never reached
}

quint8 PxAccess::referenceValue() const
{
    switch(mTraverser.channel())
    {
        case Channel::Red:
            return referenceRed();
        case Channel::Green:
            return referenceGreen();
        case Channel::Blue:
            return referenceBlue();

        default:
            qCritical("Illegal channel in rotation");
    }

    return 0; // Never reached
}

}
