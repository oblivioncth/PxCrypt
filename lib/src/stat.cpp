// Unit Includes
#include "pxcrypt/stat.h"

// Project Includes
#include "medium_io/frame.h"

/* TODO: In the long run it might make more sense to expose Frame (or canvas if it's combined with Frame)
 * to the user directly, but with PIMPL. Hide most of its function in the private class and just have
 * the few curiosity methods (like capacity) in the public one, and otherwise just use it to pass to
 * encoders/decoders for use (similar to QFile and IO classes). It can also have an isValid() method so
 * if the user wishes they can test if the frame meets the size minimum immediately after construction.
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


//-Destructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Destroys the statistics generator.
 */
Stat::~Stat() {}

/*!
 *  Returns capacity information of the image, calculated at @a bpc bits-per-channel.
 */
Stat::Capacity Stat::capacity(quint8 bpc) const
{
    Q_D(const Stat);

    // Shamelessly mirror Frame::Capacity
    PxCryptPrivate::Frame::Capacity c = PxCryptPrivate::Frame::capacity(d->mDim, bpc);
    return {.bytes = c.bytes, .leftoverBits = c.leftoverBits};
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
