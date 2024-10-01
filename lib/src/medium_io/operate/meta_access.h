#ifndef META_ACCESS_H
#define META_ACCESS_H

// Project Includes
#include "medium_io/traverse/canvas_traverser_prime.h"

namespace PxCryptPrivate
{

class MetaRef
{
    friend class MetaAccess;
//-Alias------------------------------------------------------------------------------------------------------
private:
    using ChannelRef = quint8*;
    using DisectedPixelRef = std::array<ChannelRef, 3>;

//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    DisectedPixelRef mPixelRef;

//-Constructor---------------------------------------------------------------------------------------------------------
private:
    MetaRef(const DisectedPixelRef& pixelRef);

//-Operators----------------------------------------------------------------------------------------------
public:
    quint8 operator*() const;
    void operator=(quint8 value);
};

class MetaAccess
{
//-Class Variables------------------------------------------------------------------------------------------------------
private:
    static const int META_PIXEL_COUNT = 2; // BPC + EncType

//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    CanvasTraverserPrime mTraverser;
    QRgb* mPixels;
    MetaRef mBpcRef;
    quint8 mBpcCache;
    MetaRef mEncRef;
    quint8 mEncCache;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    MetaAccess(QImage& image, const QByteArray& psk);

//-Class Functions----------------------------------------------------------------------------------------------
private:
    static quint8& channelRef(QRgb& pixel, Channel ch);

public:
    static int metaPixelCount();

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    QRgb& currentPixelRef();
    quint8& currentChannelRef();
    quint8& ncr(); // Get next channel reference

public:
    void setBpc(quint8 bpc);
    void setEnc(quint8 enc);
    quint8 bpc() const;
    quint8 enc() const;

    CanvasTraverserPrime& surrenderTraverser();
};



}

#endif // META_ACCESS_H
