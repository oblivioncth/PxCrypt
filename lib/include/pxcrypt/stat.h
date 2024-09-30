#ifndef STAT_H
#define STAT_H

// Shared Library Support
#include "pxcrypt/pxcrypt_codec_export.h"

// Qt Includes
#include <QImage>

namespace PxCrypt
{

class StatPrivate;

class PXCRYPT_CODEC_EXPORT Stat
{
    Q_DECLARE_PRIVATE(Stat);
//-Structs----------------------------------------------------------------------------------------------
public:
    struct Capacity
    {
        quint64 bytes;
        quint8 leftoverBits;
    };

//-Instance Variables----------------------------------------------------------------------------------------------
private:
    std::unique_ptr<StatPrivate> d_ptr;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    Stat(const QImage& image);
    Stat(const QSize& size);

//-Destructor---------------------------------------------------------------------------------------------------
public:
    ~Stat();

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    Capacity capacity(quint8 bpc) const;
    bool fitsMetadata() const;
    quint8 minimumDensity(quint64 bytes) const;
};

}

#endif // STAT_H
