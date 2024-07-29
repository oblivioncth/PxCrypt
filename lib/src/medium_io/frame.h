#ifndef FRAME_H
#define FRAME_H

// Project Includes
#include "pxcrypt/codec/encoder.h"
#include "medium_io/operate/basic_px_access.h"

using namespace Qt::Literals;

namespace PxCryptPrivate
{

class Frame
{
//-Aliases----------------------------------------------------------------------------------------------------------
private:
    using MetaPixelRef = std::array<std::reference_wrapper<quint8>, 3>;
    using Encoding = PxCrypt::Encoder::Encoding;

public:
    using metavalue_t = quint8;

//-Structs------------------------------------------------------------------------------------------------------
public:
    struct Capacity
    {
        quint64 bytes;
        quint8 leftoverBits;
    };

//-Class Variables----------------------------------------------------------------------------------------------
private:
    static const int META_PIXEL_COUNT = 2; // BPC + EncType
    static inline const QByteArray DEFAULT_SEED = "The best and most secure seed that is possible to exist!"_ba;

//-Instance Variables----------------------------------------------------------------------------------------------
private:
    const QImage* mImage;
    BasicPxAccess mAccess;
    MetaPixelRef mBpcPixelRef;
    MetaPixelRef mEncPixelRef;
    metavalue_t mBpcValue;
    Encoding mEncValue;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    Frame(QImage* canvas, const QByteArray& psk = {}); // TODO: Make a way for this and related classes to take const image

//-Class Functions----------------------------------------------------------------------------------------------    
private:
    static metavalue_t readMetaValue(const MetaPixelRef& pixel);
    static void writeMetaValue(const MetaPixelRef& pixel, metavalue_t value);

public:
    static int metalPixelCount();
    static bool meetsSizeMinimum(const QImage& image);
    static metavalue_t minimumBpc(const QSize& dim, quint64 bytes);
    static Capacity capacity(const QSize& dim, metavalue_t bpc);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    metavalue_t bpc() const;
    Encoding encoding() const;
    const FrameTraverser& traverser() const;
    Capacity capacity() const;

    void setBpc(metavalue_t bpc);
    void setEncoding(Encoding enc);
};

}

#endif // FRAME_H
