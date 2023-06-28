// Unit Includes
#include "pxcrypt/decode_error.h"

namespace PxCrypt
{

//===============================================================================================================
// DecodeError
//===============================================================================================================

/*!
 *  @class DecodeError <pxcrypt/decode_error.h>
 *
 *  @brief The DecodeError class is used to report errors during image decoding.
 *
 *  @sa Decoder and EncodeError.
 */

//-Class Enums-----------------------------------------------------------------------------------------------------
/*!
 *  @enum DecodeError::Type
 *
 *  This enum specifies the type of encoding error that occurred.
 *
 *  @var DecodeError::Type DecodeError::NoError
 *  No error occurred.
 *
 *  @var DecodeError::Type DecodeError::InvalidSource
 *  The encoded image was invalid.
 *
 *  @var DecodeError::Type DecodeError::MissingMedium
 *  The encoded image required a medium to be decoded but none was provided.
 *
 *  @var DecodeError::Type DecodeError::DimensionMismatch
 *  The required medium image had different dimensions than the encoded image.
 *
 *  @var DecodeError::Type DecodeError::NotLargeEnough
 *  The provided image was not large enough to be an encoded image.
 *
 *  @var DecodeError::Type DecodeError::InvalidMeta
 *  The provided image was not encoded.
 *
 *  @var DecodeError::Type DecodeError::InvalidHeader
 *  The provided image was not encoded or the password/medium were incorrect.
 *
 *  @var DecodeError::Type DecodeError::LengthMismatch
 *  The encoded image's header indicated it contained more data than possible.
 *
 *  @var DecodeError::Type DecodeError::UnexpectedEnd
 *  All pixels were skimmed before the expected payload size was reached.
 *
 *  @var DecodeError::Type DecodeError::ChecksumMismatch
 *  The payload's checksum did not match the expected value.
 */

//-Constructor-----------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an invalid DecodeError error.
 *
 *  @sa isValid().
 */
DecodeError::DecodeError() :
    mType(NoError)
{}

//Private:
DecodeError::DecodeError(Type type, QString errStr) :
    mType(type),
    mString(errStr)
{}

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
quint32 DecodeError::deriveValue() const { return static_cast<quint32>(mType); }
QString DecodeError::derivePrimary() const { return PRIMARY_STRING; }
QString DecodeError::deriveSecondary() const { return mString; }

//Public:
/*!
 *  Returns @c true if the error's type is one other than NoError; otherwise, returns @c false.
 *
 *  @sa Type.
 */
bool DecodeError::isValid() const { return mType != NoError; }

/*!
 *  Returns the type of decoding error.
 *
 *  @sa errorString().
 */
DecodeError::Type DecodeError::type() const { return mType; }

/*!
 *  Returns the string representation of the error.
 *
 *  @sa type().
 */
QString DecodeError::errorString() const { return mString; }
}
