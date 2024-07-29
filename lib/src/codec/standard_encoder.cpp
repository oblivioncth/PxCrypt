// Unit Includes
#include "pxcrypt/codec/standard_encoder.h"

// Project Includes
#include "codec/encdec.h"
#include "codec/encoder_p.h"
#include "codec/encdec.h"
#include "medium_io/frame.h"
#include "medium_io/canvas.h"
#include "art_io/works/standard.h"

namespace PxCrypt
{
/*! @cond */

//===============================================================================================================
// StandardEncoderPrivate
//===============================================================================================================

class StandardEncoderPrivate : public EncoderPrivate
{
//-Instance Variables----------------------------------------------------------------------------------------------
public:
    // Data
    QByteArray mTag;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    StandardEncoderPrivate();

//-Class Functions---------------------------------------------------------------------------------------------
public:
    static StandardEncoder::Error fromArtworkError(const PxCryptPrivate::ArtworkError aError);
};

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
StandardEncoderPrivate::StandardEncoderPrivate() :
    mTag()
{}

//-Class Functions---------------------------------------------------------------------------------------------
//Public:
StandardEncoder::Error StandardEncoderPrivate::fromArtworkError(const PxCryptPrivate::ArtworkError aError)
{
    if(!aError)
        return StandardEncoder::Error();

    QString spec = ENUM_NAME(aError.type());
    QString details = aError.details();
    if(!details.isEmpty())
        spec += u": "_s + details;

    return StandardEncoder::Error(StandardEncoder::Error::WeaveFailed, spec);
}

/*! @endcond */

//===============================================================================================================
// StandardEncoder
//===============================================================================================================

/*!
 *  @class StandardEncoder <pxcrypt/codec/standard_encoder.h>
 *
 *  @brief The StandardEncoder class encodes a payload and identifying tag into a single image.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an encoder with default settings:
 *  - BPC of 1
 *  - Empty pre-shared key
 *  - Absolute encoding
 *  - Empty tag
 */
StandardEncoder::StandardEncoder() : Encoder(std::make_unique<StandardEncoderPrivate>()) {}

//-Class Functions-------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the maximum number of payload bytes that can be stored within an image of dimensions @a dim and tag of size
 *  @a tagSize when using @a bpc bits per channel.
 *
 *  @sa calculateOptimalDensity().
 */
quint64 StandardEncoder::calculateMaximumPayload(const QSize& dim, quint16 tagSize, quint8 bpc)
{
    PxCryptPrivate::StandardWork::Measure m(tagSize, 0); // 0 size payload to check for leftover
    PxCryptPrivate::Frame::Capacity c = PxCryptPrivate::Frame::capacity(dim, bpc);
    return c.bytes - m.size();
}

/*!
 *  Returns the minimum number of bits per channel required to store a payload of @a payloadSize along with a tag
 *  of @a tagSize within a medium image of @a dim dimensions. @c 0 is returned if the payload/tag cannot fit within
 *  those dimensions.
 *
 *  Using the lowest BPC necessary is optimal as it will produce in the least distortion per pixel and the most
 *  even spread of the encoded data, overall resulting in the most minimal visual disturbance of the original image.
 *
 *  @sa calculateMaximumStorage().
 */
quint8 StandardEncoder::calculateOptimalDensity(const QSize& dim, quint16 tagSize, quint32 payloadSize)
{
    PxCryptPrivate::StandardWork::Measure m(tagSize, payloadSize);
    return m.minimumBpc(dim);
}

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the encoding tag.
 *
 *  @sa setTag().
 */
QString StandardEncoder::tag() const { Q_D(const StandardEncoder); return d->mTag; }

/*!
 *  Sets the encoding tag to @a tag.
 *
 *  Payloads are coupled with the tag during encoding in order to aid in payload identification or allow
 *  for the storage of any other string-based metadata. The tag is stored as part of the encrypted stream,
 *  not in plain text.
 *
 *  The tag is optional and can be left empty.
 *
 *  @sa tag() and encode().
 */
void StandardEncoder::setTag(const QByteArray& tag) { Q_D(StandardEncoder); d->mTag = tag; }

/*!
 *  Encodes @a payload within the medium image @a medium and stores the result in @a encoded, then
 *  returns an error status.
 *
 *  The encoding is carried out in accordance with the current configuration of the encoder.
 *
 *  If the current BPC of the encoder is @c 0, the best BPC for the given medium and payload will be determined
 *  automatically. Then, the encoder's BPC is set to that ideal value.
 *
 *  The encoded image will always use the format `QImage::Format_ARGB32` or `QImage::Format_RGB32`
 *  (depending on if @a medium has an alpha channel) regardless of the format of @a medium.
 *
 *  @warning
 *  @parblock
 *  The image produced by this function must never undergo a reduction in fidelity in order to remain
 *  decodable. In other words, it must be stored in a format that uses 8 bits per color channel and holds at least
 *  the RGB channels, else the encoded data be corrupted. No data is encoded within the alpha channel so that can
 *  safely be discarded.
 *
 *  Conversion to a format with a higher bit-depth can still result in data corruption due to aliasing sustained
 *  while resampling.
 *
 *  A 32-bit PNG is generally recommended as an ideal container.
 *  @endparblock
 *
 *  @sa StandardDecoder::decode().
 */
StandardEncoder::Error StandardEncoder::encode(QImage& encoded, QByteArrayView payload, const QImage& medium)
{
    using namespace PxCryptPrivate;

    Q_D(StandardEncoder);

    // Clear return buffer
    encoded = {};

    // Ensure data was provided
    if(payload.isEmpty())
        return Error(Error::MissingPayload);

    // Ensure bits-per-channel is valid (NOTE: optionally could clamp instead)
    if(d->mBpc > BPC_MAX)
        return Error(Error::InvalidBpc);

    // Ensure image is valid
    if(medium.isNull())
        return Error(Error::InvalidImage);

    // Measurements
    StandardWork::Measure measurement(d->mTag.size(), payload.size());

    if(d->mBpc == 0)// Determine BPC if auto
    {
        d->mBpc = measurement.minimumBpc(medium.size());
        if(d->mBpc == 0)
        {
            // Check how short at max density
            quint64 max = Frame::capacity(medium.size(), BPC_MAX).bytes;
            return Error(Error::WontFit, u"(%1 KiB short)."_s.arg((measurement.size() - max)/1024.0, 0, 'f', 3));
        }
    }
    else // Ensure data will fit with fixed BPC
    {
        quint64 max = Frame::capacity(medium.size(), d->mBpc).bytes;
        if(static_cast<quint64>(measurement.size()) > max)
            return Error(Error::WontFit, u"(%1 KiB short)."_s.arg((measurement.size() - max)/1024.0, 0, 'f', 3));
    }

    // Copy base image, normalize to standard format
    QImage workspace = standardizeImage(medium);

    // Setup frame, mark meta pixels
    Frame frame(&workspace, d->mPsk);
    frame.setBpc(d->mBpc);
    frame.setEncoding(d->mEncoding);

    // Setup IO, use self to denote relative encoding if applicable
    Canvas canvas(&workspace, frame, d->mEncoding == Relative ? &workspace : nullptr);
    canvas.open(QIODevice::WriteOnly); // Closes upon destruction

    // Write
    StandardWork work(d->mTag, payload.toByteArray());
    ArtworkError wErr = work.writeToCanvas(canvas);
    if(wErr)
        return d->fromArtworkError(wErr);

    encoded = workspace;

    return Error();
}

//===============================================================================================================
// StandardEncoder::Error
//===============================================================================================================

/*!
 *  @class StandardEncoder::Error <pxcrypt/codec/standard_encoder.h>
 *
 *  @brief The StandardEncoder::Error class is used to report errors during image encoding.
 *
 *  @sa Encoder and DecodeError.
 */

//-Class Enums-----------------------------------------------------------------------------------------------------
/*!
 *  @enum StandardEncoder::Error::Type
 *
 *  This enum specifies the type of encoding error that occurred.
 *
 *  @var StandardEncoder::Error::Type StandardEncoder::Error::NoError
 *  No error occurred.
 *
 *  @var StandardEncoder::Error::Type StandardEncoder::Error::MissingPayload
 *  No payload data was provided.
 *
 *  @var StandardEncoder::Error::Type StandardEncoder::Error::InvalidImage
 *  The medium was invalid.
 *
 *  @var StandardEncoder::Error::Type StandardEncoder::Error::WontFit
 *  The medium's dimensions were not large enough to fit the payload.
 *
 *  @var StandardEncoder::Error::Type StandardEncoder::Error::InvalidBpc
 *  The BPC value was not between 1 and 7.
 *
 *  @var StandardEncoder::Error::Type StandardEncoder::Error::WeaveFailed
 *  An unexpected error occurred while weaving data into the medium.
 */

//-Constructor-------------------------------------------------------------
//Public:
/*!
 *  Constructs an error of type @a t, with specific information @c s.
 *
 *  @sa isValid().
 */
StandardEncoder::Error::Error(Type t, const QString& s) :
    mType(t),
    mSpecific(s)
{}

//-Instance Functions-------------------------------------------------------------
//Private:
quint32 StandardEncoder::Error::deriveValue() const { return mType; }
QString StandardEncoder::Error::derivePrimary() const { return PREFIX_STRING + ' ' + ERR_STRINGS.value(mType); }
QString StandardEncoder::Error::deriveSecondary() const { return mSpecific; }

//Public:
/*!
 *  Returns @c true if the error's type is one other than NoError; otherwise, returns @c false.
 *
 *  @sa Type.
 */
bool StandardEncoder::Error::isValid() const { return mType != NoError; }

/*!
 *  Returns the type of encoding error.
 *
 *  @sa errorString().
 */
StandardEncoder::Error::Type StandardEncoder::Error::type() const { return mType; }

/*!
 *  Returns instance specific error information, if any.
 *
 *  @sa errorString().
 */
QString StandardEncoder::Error::specific() const { return mSpecific; }

/*!
 *  Returns the string representation of the error.
 *
 *  @sa type().
 */
QString StandardEncoder::Error::errorString() const { return derivePrimary() + (mSpecific.isEmpty() ? "" : ' ' + mSpecific); }

}
