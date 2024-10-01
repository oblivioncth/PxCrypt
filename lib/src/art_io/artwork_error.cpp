// Unit Include
#include "artwork_error.h"

namespace PxCryptPrivate
{

//===============================================================================================================
// ArtworkError
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
ArtworkError::ArtworkError(Type t, const QString& d) :
    mType(t),
    mDetails(d)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Public:
bool ArtworkError::isValid() const { return mType > NoError; }
ArtworkError::Type ArtworkError::type() const { return mType; };
QString ArtworkError::details() const { return mDetails; }

}
