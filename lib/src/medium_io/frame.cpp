// Unit Include
#include "frame.h"

namespace PxCryptPrivate
{

//===============================================================================================================
// Frame
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
Frame::Frame(QImage* image, const QByteArray& psk) :
    mImage(image),
    mAccess(image, !psk.isEmpty() ? psk : DEFAULT_SEED),
    mBpcPixelRef({mAccess++, mAccess++, mAccess++}),
    mEncPixelRef({mAccess++, mAccess++, mAccess++}),
    mBpcValue(readMetaValue(mBpcPixelRef)),
    mEncValue(static_cast<Encoding>(readMetaValue(mEncPixelRef)))
{
    //Q_ASSERT(image); Too late to check
}

//-Class Functions----------------------------------------------------------------------------------------------
//Private:
Frame::metavalue_t Frame::readMetaValue(const MetaPixelRef& pixel)
{
    quint8 v = 0;
    for(int i = 0; i < 3; i++)
        v |= (pixel[i].get() & 0b1) << i;
    return v;
}

void Frame::writeMetaValue(const MetaPixelRef& pixel, metavalue_t value)
{
    for(int i = 0; i < 3; i++)
    {
        if((value >> i) & 0b1)
            pixel[i].get() |= 0b1;
        else
            pixel[i].get() &= ~0b1;
    }
}

//Public:
bool Frame::meetsSizeMinimum(const QImage& image) { return image.size().width() * image.size().height() >= META_PIXEL_COUNT;  }

Frame::metavalue_t Frame::minimumBpc(const QSize& dim, quint64 bytes)
{
    // Returns the minimum BPC required to store `bytes`, or 0 if they will not fit
    if(dim.width() == 0 || dim.height() == 0)
        return 0;

    double bits = bytes * 8.0;
    double chunks = (dim.width() * dim.height() - META_PIXEL_COUNT) * 3.0;
    double bpc = std::ceil(bits/chunks);

    return bpc < 8 ? bpc : 0;
}

Frame::Capacity Frame::capacity(const QSize& dim, metavalue_t bpc)
{
    quint64 usablePixels = dim.width() * dim.height() - META_PIXEL_COUNT;
    quint64 usableChanels = usablePixels * 3;
    quint64 useableBits = usableChanels * bpc;
    quint64 useableBytes = useableBits / 8;
    quint8 leftover = useableBits % 8;

    return {.bytes = useableBytes, .leftoverBits = leftover};
}

int Frame::metalPixelCount() { return META_PIXEL_COUNT; }

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
Frame::metavalue_t Frame::bpc() const { return mBpcValue; }
Frame::Encoding Frame::encoding() const { return mEncValue; }
const FrameTraverser& Frame::traverser() const { return mAccess.traverser(); }
Frame::Capacity Frame::capacity() const { return capacity(mImage->size(), mBpcValue); }

void Frame::setBpc(metavalue_t bpc)
{
    mBpcValue = bpc;
    writeMetaValue(mBpcPixelRef, mBpcValue);
}

void Frame::setEncoding(Encoding enc)
{
    mEncValue = enc;
    writeMetaValue(mEncPixelRef, enc);
}

}
