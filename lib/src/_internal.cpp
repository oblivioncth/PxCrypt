// Unit Includes
#include "pxcrypt/_internal.h"

// Project Includes
#include "encdec_p.h"

namespace _PxCrypt
{
//-Namespace Functions-------------------------------------------------------------------------------------------------
/*!
 *  @warning This functions exists primarily for testing the library. The practical amount of storage
 *  available within an image is always lower than this. Use calculateMaximumStorage() instead.
 *
 *  Returns the maximum number of bits that can be stored within an image of dimensions @a dim and tag of size
 *  @a tagSize when using @a bpc bits per channel.
 */
quint64 calculateMaximumPayloadBits(const QSize& dim, quint16 tagSize, quint8 bpc)
{
    return PxCrypt::calcMaxPayloadBits(dim, tagSize, bpc);
}

}
