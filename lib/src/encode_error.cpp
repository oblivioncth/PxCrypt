// Unit Includes
#include "pxcrypt/encode_error.h"

namespace PxCrypt
{

//===============================================================================================================
// EncodeError
//===============================================================================================================

/*!
 *  @class EncodeError <pxcrypt/encode_error.h>
 *
 *  @brief The EncodeError class is used to report errors during image encoding.
 *
 *  @sa Encoder and DecodeError.
 */

//-Class Enums-----------------------------------------------------------------------------------------------------
/*!
 *  @enum EncodeError::Type
 *
 *  This enum specifies the type of encoding error that occurred.
 *
 *  @var EncodeError::Type EncodeError::NoError
 *  No error occurred.
 *
 *  @var EncodeError::Type EncodeError::MissingPayload
 *  No payload data was provided.
 *
 *  @var EncodeError::Type EncodeError::InvalidImage
 *  The medium was invalid.
 *
 *  @var EncodeError::Type EncodeError::WontFit
 *  The medium's dimensions were not large enough to fit the payload.
 *
 *  @var EncodeError::Type EncodeError::InvalidBpc
 *  The BPC value was not between 1 and 7.
 */

//-Constructor-----------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an invalid encoder error.
 *
 *  @sa isValid().
 */
EncodeError::EncodeError() :
    mType(NoError)
{}

//Private:
EncodeError::EncodeError(Type type, QString errStr) :
    mType(type),
    mString(errStr)
{}

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
quint32 EncodeError::deriveValue() const { return static_cast<quint32>(mType); }
QString EncodeError::derivePrimary() const { return PRIMARY_STRING; }
QString EncodeError::deriveSecondary() const { return mString; }

//Public:
/*!
 *  Returns @c true if the error's type is one other than NoError; otherwise, returns @c false.
 *
 *  @sa Type.
 */
bool EncodeError::isValid() const { return mType != NoError; }

/*!
 *  Returns the type of encoding error.
 *
 *  @sa errorString().
 */
EncodeError::Type EncodeError::type() const { return mType; }

/*!
 *  Returns the string representation of the error.
 *
 *  @sa type().
 */
QString EncodeError::errorString() const { return mString; }
}
