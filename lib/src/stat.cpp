// Unit Includes
#include "pxcrypt/stat.h"

// Project Includes
#include "medium_io/operate/meta_access.h"

/* TODO: In the long run it might make more sense to expose Canvas to the user directly, but with PIMPL.
 * Hide most of its function in the private class and just have the few curiosity methods (like capacity)
 * in the public one, and otherwise just use it to pass to encoders/decoders for use (similar to QFile
 * and IO classes). It can also have an isValid() method so if the user wishes they can test if the
 * frame meets the size minimum immediately after construction.
 *
 * This change would likely make this class unnecessary.
 */

namespace PxCrypt
{
/*! @cond */

//===============================================================================================================
// StatPrivate
//===============================================================================================================

class StatPrivate
{
//-Instance Variables----------------------------------------------------------------------------------------------
public:
    QSize mDim;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    StatPrivate();
};

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
StatPrivate::StatPrivate() :
    mDim()
{}

/*! @endcond */

//===============================================================================================================
// Stat
//===============================================================================================================

/*!
 *  @class Stat <pxcrypt/stat.h>
 *
 *  @brief The Stat provides the means to generate various statistics of a potential PxCrypt image.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a statistics generator for @a image.
 */
Stat::Stat(const QImage& image) :
    d_ptr(std::make_unique<StatPrivate>())
{
    Q_D(Stat);
    d->mDim = image.size();
}

/*!
 *  Constructs a statistics generator for an image of size @a size.
 */
Stat::Stat(const QSize& size) :
    d_ptr(std::make_unique<StatPrivate>())
{
    Q_D(Stat);
    d->mDim = size;
}


//-Destructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Destroys the statistics generator.
 */
Stat::~Stat() {}

//-Instance Functions-------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns capacity information of the image, calculated at @a bpc bits-per-channel.
 */
Stat::Capacity Stat::capacity(quint8 bpc) const
{
    Q_D(const Stat);

    quint64 usablePixels = d->mDim.width() * d->mDim.height() - PxCryptPrivate::MetaAccess::metaPixelCount();
    quint64 usableChanels = usablePixels * 3;
    quint64 useableBits = usableChanels * bpc;
    quint64 useableBytes = useableBits / 8;
    quint8 leftover = useableBits % 8;

    return {.bytes = useableBytes, .leftoverBits = leftover};
}

/*!
 *  Returns @c true if the image is large enough to fit the standard metadata required by every type of
 *  PxCrypt image; otherwise, returns @c false.
 */
bool Stat::fitsMetadata() const
{
    Q_D(const Stat);

    return d->mDim.width() * d->mDim.height() >= PxCryptPrivate::MetaAccess::metaPixelCount();
}
/*!
 *  Returns @c the smallest density (bits-per-channel) requires to store @a bytes total bytes within
 *  the image, regardless of encoder underlying storage format, or @c 0 if @a bytes exceeds the capacity
 *  of the image.
 *
 *  @sa capacity().
 */
quint8 Stat::minimumDensity(quint64 bytes) const
{
    Q_D(const Stat);

    if(d->mDim.width() == 0 || d->mDim.height() == 0)
        return 0;

    double bits = bytes * 8.0;
    double chunks = (d->mDim.width() * d->mDim.height() - PxCryptPrivate::MetaAccess::metaPixelCount()) * 3.0;
    double bpc = std::ceil(bits/chunks);

    return bpc < 8 ? bpc : 0;
}

//===============================================================================================================
// Stat::Capacity
//===============================================================================================================

/*!
 *  @struct Stat::Capacity <pxcrypt/stat.h>
 *
 *  @brief The Capacity holds information about the capacity of an image at a given BPC.
 *
 *  @var quint64 Stat::Capacity::bytes
 *  The total number of bytes the image can hold, regardless of encoder or underlying storage format.
 *
 *  @var quint64 Stat::Capacity::leftoverBits
 *  The number of "dead" (unusable) bits that are left over after the final full-byte-boundary.
 */

}
