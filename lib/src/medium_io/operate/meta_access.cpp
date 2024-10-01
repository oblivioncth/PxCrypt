// Unit Include
#include "meta_access.h"

namespace PxCryptPrivate
{

//===============================================================================================================
// MetaRef
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Private:
MetaRef::MetaRef(const DisectedPixelRef& pixelRef) :
    mPixelRef(pixelRef)
{}

//-Operators----------------------------------------------------------------------------------------------
//Private:
quint8 MetaRef::operator*() const
{
    quint8 v = 0;
    for(int i = 0; i < 3; i++)
        v |= (*mPixelRef[i] & 0b1) << i;
    return v;
}

void MetaRef::operator=(quint8 value)
{
    for(int i = 0; i < 3; i++)
    {
        if((value >> i) & 0b1)
            *mPixelRef[i] |= 0b1;
        else
            *mPixelRef[i] &= ~0b1;
    }
}

//===============================================================================================================
// MetaAccess
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
MetaAccess::MetaAccess(QImage& image, const QByteArray& psk) :
    mTraverser(image, psk),
    mPixels(reinterpret_cast<QRgb*>(image.bits())),
    mBpcRef({&ncr(), &ncr(), &ncr()}),
    mBpcCache(*mBpcRef),
    mEncRef({&ncr(), &ncr(), &ncr()}),
    mEncCache(*mEncRef)
{
    // TODO: Have this check or a more complete one in Canvas cause this occurs too late due to initializers
    //Q_ASSERT(image.width() * image.height() >= META_PIXEL_COUNT);
}

//-Class Functions------------------------------------------------------------------------------------------------
//Private:
quint8& MetaAccess::channelRef(QRgb& pixel, Channel ch)
{
    // As of Qt 6.7, QRgb is brazenly defined as 'unsigned int', despite that not being guaranteed to be 32-bit, so make sure
    static_assert(sizeof(pixel) == 4);

    // Dirty direct memory access, but meh, base it on system endianness and we're good
    quint8* chPtr = reinterpret_cast<quint8*>(&pixel);
    if constexpr(QSysInfo::ByteOrder == QSysInfo::BigEndian) // 0xAARRGGBB
        chPtr += ch;
    else // 0xBBGGRRAA
        chPtr += (3 - ch);

    return *chPtr;
}

//Public:
int MetaAccess::metaPixelCount() { return META_PIXEL_COUNT; }

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
QRgb& MetaAccess::currentPixelRef() { return mPixels[mTraverser.pixelIndex()]; }
quint8& MetaAccess::currentChannelRef() { return channelRef(currentPixelRef(), mTraverser.channel()); }
quint8& MetaAccess::ncr()
{
    quint8& v = currentChannelRef();
    mTraverser.nextChannel();
    return v;
}

//Public:
void MetaAccess::setBpc(quint8 bpc)
{
    mBpcCache = bpc;
    mBpcRef = bpc;
}

void MetaAccess::setEnc(quint8 enc)
{
    mEncCache = enc;
    mEncRef = enc;
}

quint8 MetaAccess::bpc() const { return mBpcCache; }
quint8 MetaAccess::enc() const { return mEncCache; }
CanvasTraverserPrime& MetaAccess::surrenderTraverser() { return mTraverser; }

}
