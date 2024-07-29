#ifndef BASIC_PX_ACCESS_H
#define BASIC_PX_ACCESS_H

// Project Includes
#include "medium_io/traverse/frame_traverser.h"

namespace PxCryptPrivate
{

class BasicPxAccess
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QRgb* mPixels;
    FrameTraverser mTraverser;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    BasicPxAccess(QImage* image, const QByteArray& psk);

//-Class Functions----------------------------------------------------------------------------------------------
private:
    // Right now this is only used with non-const references, but the flexibility is there for the future
    template<typename RgbT, typename ChT = std::conditional_t<std::is_const_v<RgbT>, const quint8, quint8>>
        requires std::same_as<std::remove_const_t<RgbT>, QRgb>
    static ChT& channelRef(RgbT& pixel, Channel ch);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    const QRgb& constFramePixel() const;
    QRgb& framePixel();

public:
    quint8 constValue() const;
    quint8& value();
    const FrameTraverser& traverser() const;

    void nextChannel();

//-Operators----------------------------------------------------------------------------------------------------------------
public:
    quint8& operator++(int); // value() + nextChannel()
};

}

#endif // BASIC_PX_ACCESS_H
