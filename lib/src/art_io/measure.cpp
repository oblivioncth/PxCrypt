// Project Includes
#include "art_io/artwork.h"

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
Frame::metavalue_t IMeasure::minimumBpc(const QSize& dim) const { return Frame::minimumBpc(dim, size()); }

}
