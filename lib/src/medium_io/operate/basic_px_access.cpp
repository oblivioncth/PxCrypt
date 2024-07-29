// Unit Include
#include "basic_px_access.h"

// Project Includes
#include "medium_io/frame.h"

namespace PxCryptPrivate
{

//===============================================================================================================
// PxAccess
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
BasicPxAccess::BasicPxAccess(QImage* image, const QByteArray& psk) :
    mPixels(reinterpret_cast<QRgb*>(image->bits())),
    mTraverser(*image, psk)
{
    Q_ASSERT(image->width() * image->height() >= Frame::metalPixelCount()); // Practically this is too small, but it is the minimum for this class to work
}

//-Class Functions------------------------------------------------------------------------------------------------
//Private:
template<typename RgbT, typename ChT>
    requires std::same_as<std::remove_const_t<RgbT>, QRgb>
ChT& BasicPxAccess::channelRef(RgbT& pixel, Channel ch)
{
    // As of Qt 6.7, QRgb is brazenly defined as 'unsigned int', despite that not being guaranteed to be 32-bit, so make sure
    static_assert(sizeof(pixel) == 4);

    // Dirty direct memory access, but meh, base it on system endianness and we're good
    ChT* chPtr = reinterpret_cast<ChT*>(&pixel);
    if constexpr(QSysInfo::ByteOrder == QSysInfo::BigEndian) // 0xAARRGGBB
        chPtr += ch;
    else // 0xBBGGRRAA
        chPtr += (3 - ch);

    return *chPtr;
}
//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
QRgb& BasicPxAccess::framePixel() { return mPixels[mTraverser.pixelIndex()]; }
const QRgb& BasicPxAccess::constFramePixel() const { return mPixels[mTraverser.pixelIndex()]; }

//Public:
quint8 BasicPxAccess::constValue() const { return channelRef(constFramePixel(), mTraverser.channel()); }
quint8& BasicPxAccess::value() { return channelRef(framePixel(), mTraverser.channel()); }
const FrameTraverser& BasicPxAccess::traverser() const { return mTraverser; }
void BasicPxAccess::nextChannel() { mTraverser.nextChannel(); }

//-Operators----------------------------------------------------------------------------------------------------------------
//Public:
quint8& BasicPxAccess::operator++(int)
{
    quint8& v = value();
    nextChannel();
    return v;
}

}
