// Unit Includes
#include "pxcrypt/codec/standard_decoder.h"

// Project Includes
#include "pxcrypt/codec/encoder.h"
#include "codec/decoder_p.h"
#include "codec/encdec.h"
#include "medium_io/canvas.h"
#include "art_io/works/standard.h"
#include "pxcrypt/stat.h"

namespace PxCrypt
{
/*! @cond */

//===============================================================================================================
// StandardDecoderPrivate
//===============================================================================================================

class StandardDecoderPrivate : public DecoderPrivate
{
//-Instance Variables--------------------------------------------------------------------------------------------
public:
    QString mTag;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    StandardDecoderPrivate();

//-Class Functions---------------------------------------------------------------------------------------------
public:
    static StandardDecoder::Error fromArtworkError(const PxCryptPrivate::ArtworkError& aError);
};

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
StandardDecoderPrivate::StandardDecoderPrivate() :
    mTag()
{}

//-Instance Functions---------------------------------------------------------------------------------------------
//Public:
StandardDecoder::Error StandardDecoderPrivate::fromArtworkError(const PxCryptPrivate::ArtworkError& aError)
{
    if(!aError)
        return StandardDecoder::Error();

    QString spec = ENUM_NAME(aError.type());
    QString details = aError.details();
    if(!details.isEmpty())
        spec += u": "_s + details;

    return StandardDecoder::Error(StandardDecoder::Error::SkimFailed, spec);
}

/*! @endcond */

//===============================================================================================================
// StandardDecoder
//===============================================================================================================

/*!
 *  @class StandardDecoder <pxcrypt/codec/standard_decoder.h>
 *
 *  @brief The StandardDecoder class decodes a payload and identifying tag from a single encoded image.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a decoder with an empty pre-shared key.
 */
StandardDecoder::StandardDecoder() : Decoder(std::make_unique<StandardDecoderPrivate>()) {}

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the tag of the last successfully decoded payload.
 *
 *  @sa Encoder::setTag().
 */
QString StandardDecoder::tag() const { Q_D(const StandardDecoder); return d->mTag; }

//TODO: Make decoded and tag the only args (as buffers) here and have the rest moved to setters/getters, do the
//equivalent in Encoder
/*!
 *  Retrieves data from the encoded PxCrypt image @a encoded and stores the result in @a decoded, then
 *  returns an error status.
 *
 *  The original medium image @a medium is used as a reference for images with encodings that require it, but
 *  is otherwise ignored. The encoded data is descrambled using the currently set pre-shared key.
 *
 *  After an image is successfully decoded the tag of the encoded data is made available via tag().
 *
 *  @sa StandardEncoder::encode().
 */
StandardDecoder::Error StandardDecoder::decode(QByteArray& decoded, const QImage& encoded, const QImage& medium)
{
    using namespace PxCryptPrivate;

    Q_D(StandardDecoder);

    // Clear return buffer
    decoded.clear();

    // Ensure encoded image is valid
    if(encoded.isNull())
        return Error(Error::InvalidSource);

    // Get image stats
    Stat encStat(encoded);

    // Ensure image meets bare minimum space for meta pixels
    if(!encStat.fitsMetadata())
        return Error(Error::NotLargeEnough);

    // Ensure standard pixel format
    QImage encStd = standardizeImage(encoded);

    // Setup canvas
    Canvas canvas(encStd, d->mPsk);

    // Ensure BPC is valid
    quint8 bpc = canvas.bpc();
    if(bpc < BPC_MIN || bpc > BPC_MAX)
        return Error(Error::InvalidMeta);

    // Ensure encoding is valid
    Encoder::Encoding encoding = canvas.encoding();
    if(!magic_enum::enum_contains(encoding))
        return Error(Error::InvalidMeta);

    // Bare minimum size check
    Stat::Capacity capacity = encStat.capacity(bpc);
    quint64 minSize = StandardWork::Measure().size();
    if(capacity.bytes < minSize)
        return Error(Error::NotLargeEnough);

    // Ensure medium image is valid if applicable
    QImage mediumStd;
    if(encoding == Encoder::Relative)
    {
        if(medium.isNull())
            return Error(Error::MissingMedium);

        if(medium.size() != encStd.size())
            return Error(Error::DimensionMismatch);

        mediumStd = standardizeImage(medium);
        canvas.setReference(&mediumStd);
    }

    // Prepare for IO
    canvas.open(QIODevice::ReadOnly); // Closes upon destruction

    // Read
    StandardWork work;
    ArtworkError rErr = StandardWork::readFromCanvas(work, canvas);
    if(rErr)
        return d->fromArtworkError(rErr);

    d->mTag = work.tag(); // Only store tags from successfully decoded images
    decoded = work.payload();

    return Error();
}

//===============================================================================================================
// StandardDecoder::Error
//===============================================================================================================

/*!
 *  @class StandardDecoder::Error <pxcrypt/codec/standard_decoder.h>
 *
 *  @brief The StandardDecoder::Error class is used to report errors during image decoding.
 *
 *  @sa StandardDecoder and StandardDecoder::Error.
 */

//-Class Enums-----------------------------------------------------------------------------------------------------
/*!
 *  @enum StandardDecoder::Error::Type
 *
 *  This enum specifies the type of encoding error that occurred.
 *
 *  @var StandardDecoder::Error::Type StandardDecoder::Error::NoError
 *  No error occurred.
 *
 *  @var StandardDecoder::Error::Type StandardDecoder::Error::InvalidSource
 *  The encoded image was invalid.
 *
 *  @var StandardDecoder::Error::Type StandardDecoder::Error::MissingMedium
 *  The encoded image required a medium to be decoded but none was provided.
 *
 *  @var StandardDecoder::Error::Type StandardDecoder::Error::DimensionMismatch
 *  The required medium image had different dimensions than the encoded image.
 *
 *  @var StandardDecoder::Error::Type StandardDecoder::Error::NotLargeEnough
 *  The provided image was not large enough to be an encoded image.
 *
 *  @var StandardDecoder::Error::Type StandardDecoder::Error::InvalidMeta
 *  The provided image was not encoded.
 *
 *  @var StandardDecoder::Error::Type StandardDecoder::Error::SkimFailed
 *  An unexpected error occurred while skimming data from the image.
 */

//-Constructor-------------------------------------------------------------
//Public:
/*!
 *  Constructs an error of type @a t, with specific information @c s.
 *
 *  @sa isValid().
 */
StandardDecoder::Error::Error(Type t, const QString& s) :
    mType(t),
    mSpecific(s)
{}

//-Instance Functions-------------------------------------------------------------
//Private:
quint32 StandardDecoder::Error::deriveValue() const { return mType; }
QString StandardDecoder::Error::derivePrimary() const { return PREFIX_STRING + ' ' + ERR_STRINGS.value(mType); }
QString StandardDecoder::Error::deriveSecondary() const { return mSpecific; }

//Public:
/*!
 *  Returns @c true if the error's type is one other than NoError; otherwise, returns @c false.
 *
 *  @sa Type.
 */
bool StandardDecoder::Error::isValid() const { return mType != NoError; }

/*!
 *  Returns the type of decoding error.
 *
 *  @sa errorString().
 */
StandardDecoder::Error::Type StandardDecoder::Error::type() const { return mType; }

/*!
 *  Returns instance specific error information, if any.
 *
 *  @sa errorString().
 */
QString StandardDecoder::Error::specific() const { return mSpecific; }

/*!
 *  Returns the string representation of the error.
 *
 *  @sa type().
 */
QString StandardDecoder::Error::errorString() const { return derivePrimary() + (mSpecific.isEmpty() ? "" : ' ' + mSpecific); }

}
