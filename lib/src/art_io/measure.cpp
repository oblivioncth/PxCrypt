// Unit Include
#include "measure.h"

// Project Includes
#include "art_io/artwork.h"
#include "pxcrypt/stat.h"

namespace PxCryptPrivate
{

//===============================================================================================================
// IMeasure
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Protected:
IMeasure::IMeasure() {}

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
quint64 IMeasure::size() const { return IArtwork::size(renditionSize()); }
Canvas::metavalue_t IMeasure::minimumBpc(const QSize& dim) const { return PxCrypt::Stat(dim).minimumDensity(size()); }
quint64 IMeasure::leftOverSpace(const QSize& dim, Canvas::metavalue_t bpc) const
{
    return PxCrypt::Stat(dim).capacity(bpc).bytes - size();
}

}
