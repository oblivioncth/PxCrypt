// Unit Includes
#include "pxcrypt/codec/multi_decoder.h"

// Standard Library Includes
#include <algorithm>
#include <execution>

// Qt Includes
#include <QtConcurrent>

// Qx Includes
#include <qx/core/qx-integrity.h>

// Project Includes
#include "codec/decoder_p.h"
#include "art_io/works/multipart.h"
#include "pxcrypt/stat.h"

using namespace PxCryptPrivate;
using Measure = MultiPartWork::Measure;
using Error = PxCrypt::MultiDecoder::Error;

namespace PxCrypt
{
/*! @cond */

// Wraps Error for use with QtConcurrent
class MultiDecoderException : public QException
{
    Error mError;
public:
    MultiDecoderException() noexcept = default;
    MultiDecoderException(const Error& err) : mError(err) {}

    void raise() const override { throw *this; }
    MultiDecoderException* clone() const override { return new MultiDecoderException(*this); }
    Error error() const { return mError; }
};

//===============================================================================================================
// MultiDecoderPrivate
//===============================================================================================================

class MultiDecoderPrivate : public DecoderPrivate
{
//-Instance Variables--------------------------------------------------------------------------------------------
public:
    QString mTag;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    MultiDecoderPrivate();

//-Class Functions---------------------------------------------------------------------------------------------
public:
    static Error fromArtworkError(const PxCryptPrivate::ArtworkError& aError, qsizetype origIdx);

//-Instance Functions--------------------------------------------------------------------------------------------
public:
    Error decode(QList<MultiPartWork>& decoded, const QList<QImage>& encoded, const QList<QImage>& mediums);
};

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
MultiDecoderPrivate::MultiDecoderPrivate() :
    mTag()
{}

//-Class Functions-----------------------------------------------------------------------------------------------
//Public:
MultiDecoder::Error MultiDecoderPrivate::fromArtworkError(const PxCryptPrivate::ArtworkError& aError, qsizetype origIdx)
{
    if(!aError)
        return MultiDecoder::Error();

    QString spec = ENUM_NAME(aError.type());
    QString details = aError.details();
    if(!details.isEmpty())
        spec += u": "_s + details;

    return MultiDecoder::Error(MultiDecoder::Error::SkimFailed, origIdx, spec);
}

//-Instance Functions---------------------------------------------------------------------------------------------
//Public:
Error MultiDecoderPrivate::decode(QList<MultiPartWork>& decoded, const QList<QImage>& encoded, const QList<QImage>& mediums)
{
    try{
        decoded = QtConcurrent::blockingMapped(encoded, [&, listStart = &encoded[0]](const QImage& i){
            /* We determine the original index by using the fact that QList stores
             * elements contiguously, in that we can take the difference between the
             * address of this image and the first.
             */
            qsizetype origIdx = std::distance(listStart, &i);

            // Ensure encoded image is valid
            if(i.isNull())
                throw MultiDecoderException(Error(Error::InvalidSource, origIdx));

            // Get image stats
            Stat encStat(i);

            // Ensure image meets bare minimum space for meta pixels
            if(!encStat.fitsMetadata())
                throw MultiDecoderException(Error(Error::NotLargeEnough, origIdx));

            // Ensure standard pixel format
            QImage iStd = standardizeImage(i);

            // Setup canvas
            Canvas canvas(iStd, mPsk);

            // Ensure BPC is valid
            quint8 bpc = canvas.bpc();
            if(bpc < BPC_MIN || bpc > BPC_MAX)
                throw MultiDecoderException(Error(Error::InvalidMeta, origIdx));

            // Ensure encoding is valid
            Encoder::Encoding encoding = canvas.encoding();
            if(!magic_enum::enum_contains(encoding))
                throw MultiDecoderException(Error(Error::InvalidMeta, origIdx));

            // Bare minimum size check
            Stat::Capacity capacity = encStat.capacity(bpc);
            quint64 minSize = MultiPartWork::Measure().size();
            if(capacity.bytes < minSize)
                throw MultiDecoderException(Error(Error::NotLargeEnough));

            // Ensure medium image is valid if applicable
            QImage mediumStd;
            if(encoding == Encoder::Relative)
            {
                if(mediums.size() != encoded.size())
                    throw MultiDecoderException(Error(Error::MissingMediums));

                auto& med = mediums[origIdx];
                if(med.isNull())
                    throw MultiDecoderException(Error(Error::MissingMediums, origIdx));

                if(med.size() != iStd.size())
                    throw MultiDecoderException(Error(Error::DimensionMismatch, origIdx));

                mediumStd = standardizeImage(med);
                canvas.setReference(&mediumStd);
            }

            // Prepare for IO
            canvas.open(QIODevice::ReadOnly); // Closes upon destruction

            // Read
            MultiPartWork work;
            if(ArtworkError rErr = MultiPartWork::readFromCanvas(work, canvas))
                throw MultiDecoderException(fromArtworkError(rErr, origIdx));

            return work;
        });
    } catch (MultiDecoderException& e) {
        return e.error();
    }

    return {};
}

/*! @endcond */

//===============================================================================================================
// MultiDecoder
//===============================================================================================================

/*!
 *  @class MultiDecoder <pxcrypt/codec/multi_decoder.h>
 *
 *  @brief The MultiDecoder class decodes a payload and identifying tag from multiple encoded images.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs a decoder with an empty pre-shared key.
 */
MultiDecoder::MultiDecoder() : Decoder(std::make_unique<MultiDecoderPrivate>()) {}

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the tag of the last successfully decoded payload.
 *
 *  @sa Encoder::setTag().
 */
QString MultiDecoder::tag() const { Q_D(const MultiDecoder); return d->mTag; }

/*!
 *  Retrieves data from the encoded set of PxCrypt images @a encoded and stores the result in @a decoded, then
 *  returns an error status.
 *
 *  The original medium images @a mediums are used as a reference for images with encodings that require it, but
 *  are otherwise ignored. The encoded data is descrambled using the currently set pre-shared key.
 *
 *  @note @a encoded and @a mediums do not need to be in the same order they were at the time of encoding, but
 *  they do need to be in the same order relative to each other so that the correct medium is matched with
 *  the correct encoded image.
 *
 *  After an image is successfully decoded the tag of the encoded data is made available via tag().
 *
 *  @sa MultiEncoder::encode().
 */
MultiDecoder::Error MultiDecoder::decode(QByteArray& decoded, const QList<QImage>& encoded, const QList<QImage>& mediums)
{
    Q_D(MultiDecoder);

    // Clear return buffer
    decoded.clear();

    if(encoded.isEmpty())
        return Error(Error::MissingSources);

    // Decode parts
    QList<MultiPartWork> works;
    if(auto err = d->decode(works, encoded, mediums))
        return err;

    // Grab some data arbitrarily from the first work to make comparisons
    const auto& fw = works.front();
    QString tag = fw.tag();
    auto completeChecksum = fw.completeChecksum();
    auto partCount = fw.partCount();

    // Ensure enough parts are present
    if(partCount != encoded.count())
        return Error(Error::PartsMissing, -1, u"Have %1 parts, need %2."_s.arg(encoded.count(), partCount));

    // Sort by part number
    std::sort(std::execution::par, works.begin(), works.end(), [](const MultiPartWork& a, const MultiPartWork& b){
        return a.partIdx() < b.partIdx();
    });

    // Build full payload and ensure correctness
    QByteArray fullPayload;
    for(qsizetype i = 0; i < works.size(); ++i)
    {
        auto& w = works.at(i);

        // Ensure part is as expected
        if(w.partIdx() != i)
            return Error(Error::PartMismatch, i, u"The part index was unexpected (incomplete set with unrelated image)."_s);
        else if(w.tag() != tag)
            return Error(Error::PartMismatch, i, u"The part tag was different than the rest."_s);
        else if(w.completeChecksum() != completeChecksum)
            return Error(Error::PartMismatch, i, u"The complete checksum was different from the rest."_s);

        // Add portion
        fullPayload.append(w.partPayload());
    }

    // Check complete checksum
    if(auto sumCheck = Qx::Integrity::crc32(fullPayload); sumCheck!= completeChecksum)
        return Error(Error::ChecksumMismatch, -1, u"The payload's checksum did not match its record."_s);

    d->mTag = tag; // Only store tags from successfully decoded images
    decoded = fullPayload;

    return Error();
}

//===============================================================================================================
// MultiDecoder::Error
//===============================================================================================================

/*!
 *  @class MultiDecoder::Error <pxcrypt/codec/multi_decoder.h>
 *
 *  @brief The MultiDecoder::Error class is used to report errors during multi-image decoding.
 *
 *  @sa MultiDecoder and MultiDecoder::Error.
 */

//-Class Enums-----------------------------------------------------------------------------------------------------
/*!
 *  @enum MultiDecoder::Error::Type
 *
 *  This enum specifies the type of encoding error that occurred.
 *
 *  @var MultiDecoder::Error::Type MultiDecoder::Error::NoError
 *  No error occurred.
 *
 *  @var MultiDecoder::Error::Type MultiDecoder::Error::MissingSources
 *  No encoded images were provided.
 *
 *  @var MultiDecoder::Error::Type MultiDecoder::Error::InvalidSource
 *  The encoded image was invalid.
 *
 *  @var MultiDecoder::Error::Type MultiDecoder::Error::MissingMediums
 *  The encoded images require mediums to be decoded but none, or not enough were provided,
 *  or some were invalid.
 *
 *  @var MultiDecoder::Error::Type MultiDecoder::Error::DimensionMismatch
 *  The required medium image had different dimensions than the encoded image.
 *
 *  @var MultiDecoder::Error::Type MultiDecoder::Error::NotLargeEnough
 *  The provided image was not large enough to be an encoded image.
 *
 *  @var MultiDecoder::Error::Type MultiDecoder::Error::InvalidMeta
 *  The provided image was not encoded.
 *
 *  @var MultiDecoder::Error::Type MultiDecoder::Error::PartMismatch
 *  One of the parts had a different complete checksum or tag than the rest, or an
 *  unexpected part index.
 *
 *  @var MultiDecoder::Error::Type MultiDecoder::Error::PartsMissing
 *  The image set consists of more parts than were provided.
 *
 *  @var MultiDecoder::Error::Type MultiDecoder::Error::ChecksumMismatch
 *  The full payload checksum did not match that of the reassembled payload.
 *
 *  @var MultiDecoder::Error::Type MultiDecoder::Error::SkimFailed
 *  An unexpected error occurred while skimming data from the image.
 */

//-Constructor-------------------------------------------------------------
//Public:
/*!
 *  Constructs an error of type @a t, with specific information @c s, related to the input image
 *  at index @a idx.
 *
 *  @note An index of @c -1 is used to indicate an error that is not tied to a specific image.
 *
 *  @sa isValid().
 */
MultiDecoder::Error::Error(Type t, qsizetype idx, const QString& s) :
    mType(t),
    mSpecific(s),
    mImageIndex(idx)
{}

//-Instance Functions-------------------------------------------------------------
//Private:
quint32 MultiDecoder::Error::deriveValue() const { return mType; }
QString MultiDecoder::Error::derivePrimary() const { return PREFIX_STRING + ' ' + ERR_STRINGS.value(mType); }
QString MultiDecoder::Error::deriveSecondary() const { return mSpecific; }

//Public:
/*!
 *  Returns @c true if the error's type is one other than NoError; otherwise, returns @c false.
 *
 *  @sa Type.
 */
bool MultiDecoder::Error::isValid() const { return mType != NoError; }

/*!
 *  Returns the type of decoding error.
 *
 *  @sa errorString().
 */
MultiDecoder::Error::Type MultiDecoder::Error::type() const { return mType; }

/*!
 *  Returns the index of the image that caused the error, or @c -1 if the error is not image specific.
 */
qsizetype MultiDecoder::Error::imageIndex() const { return mImageIndex; }

/*!
 *  Returns instance specific error information, if any.
 *
 *  @sa errorString().
 */
QString MultiDecoder::Error::specific() const { return mSpecific; }

/*!
 *  Returns the string representation of the error.
 *
 *  @sa type().
 */
QString MultiDecoder::Error::errorString() const
{
    QString str = derivePrimary();
    if(mImageIndex >= 0)
        str += '[' + QString::number(mImageIndex) + ']';
    if(!mSpecific.isEmpty())
        str += ' ' + mSpecific;
    return derivePrimary() +  (mSpecific.isEmpty() ? "" : ' ' + mSpecific);
}

}
