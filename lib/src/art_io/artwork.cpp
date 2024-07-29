// Unit Include
#include "artwork.h"

// Qt Includes
#include <QDataStream>

namespace PxCryptPrivate
{

//===============================================================================================================
// IArtwork
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Protected:
IArtwork::IArtwork() {}

//-Class Functions----------------------------------------------------------------------------------------------
//Public:
quint64 IArtwork::size(quint64 renditionSize) { return MAGIC_NUM.size() + sizeof(rendition_id_t) + renditionSize; }

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
quint64 IArtwork::size() const { return size(renditionSize()); }

}
