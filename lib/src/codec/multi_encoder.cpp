// Unit Includes
#include "pxcrypt/codec/multi_encoder.h"

// Standard Library Includes
#include <algorithm>
#include <execution>

// Qt Includes
#include <QtConcurrent>

// Qx Includes
#include <qx/core/qx-integrity.h>

// Project Includes
#include "codec/encoder_p.h"
#include "art_io/works/multipart.h"
#include "pxcrypt/stat.h"

using namespace PxCryptPrivate;
using Measure = MultiPartWork::Measure;
using Error = PxCrypt::MultiEncoder::Error;

namespace PxCrypt
{
/*! @cond */

// Wraps Error for use with QtConcurrent
class MultiEncoderException : public QException
{
    Error mError;
public:
    MultiEncoderException() noexcept = default;
    MultiEncoderException(const Error& err) : mError(err) {}

    void raise() const override { throw *this; }
    MultiEncoderException* clone() const override { return new MultiEncoderException(*this); }
    Error error() const { return mError; }
};

struct Apportionment
{
    quint64 factor;
    qsizetype origIdx;
    quint64 bytes;
    quint64 byteWeight;
    QByteArrayView slice;
    MultiPartWork::part_idx_t partIdx;
};

//===============================================================================================================
// MultiEncoderPrivate
//===============================================================================================================

class MultiEncoderPrivate : public EncoderPrivate
{
//-Instance Variables----------------------------------------------------------------------------------------------
public:
    // Data
    QByteArray mTag;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    MultiEncoderPrivate();

//-Class Functions---------------------------------------------------------------------------------------------
private:
    /* These are essentially atomic versions of
     * max = std::max(max, val);
     *
     * and
     *
     * min = std::min(min, val);
     *
     * TODO: C++26 has this natively (fetch_max/fetch_min)
     *
     * Since we can't load, compare, and store in the same op,
     * we have to fetch, compare, and then try to store. If the value has changed
     * since our fetch, we start over.
     *
     * Technically this could be "slow" as it uses the strictest memory order mode, but it's expected
     * that the old < val comparison should only succeed one or twice per run, so the impact should
     * be negligible.
     */
    template<typename T>
    T fetch_max(std::atomic<T>& max, T val)
    {
        T old = max.load(std::memory_order_relaxed);
        while(old < val && !max.compare_exchange_weak(old, val));
        return old;
    }

    template<typename T>
    T fetch_min(std::atomic<T>& min, T val)
    {
        T old = min.load(std::memory_order_relaxed);
        while(old > val && !min.compare_exchange_weak(old, val));
        return old;
    }

public:
    static Error fromArtworkError(const PxCryptPrivate::ArtworkError aError, qsizetype origIdx);

//-Instance Functions---------------------------------------------------------------------------------------------
public:
    // TODO: Simplify argument lists using a struct or temporarily storing the re-used args in this class
    Error measureAndAccumulate(quint64& factorTotal, QList<Apportionment>& measurements, const QList<QImage>& mediums);
    quint64 initialApportionment(quint64 factorTotal, quint64 bytesTotal, QList<Apportionment>& apportionments);
    void finalApportionment(quint64 remainingBytes, QByteArrayView payload, QList<Apportionment>& apportionments);
    Error encode(QByteArrayView payload, QList<QImage>& encoded, const QList<Apportionment>& finalApportionments, const QList<QImage>& mediums);
    void restoreOriginalOrder(QList<QImage>& encoded, QList<Apportionment>& apportionments);
};

//-Constructor---------------------------------------------------------------------------------------------------
//Public:
MultiEncoderPrivate::MultiEncoderPrivate() :
    mTag()
{}

//-Class Functions---------------------------------------------------------------------------------------------
//Public:
Error MultiEncoderPrivate::fromArtworkError(const PxCryptPrivate::ArtworkError aError, qsizetype origIdx)
{
    if(!aError)
        return Error();

    QString spec = ENUM_NAME(aError.type());
    QString details = aError.details();
    if(!details.isEmpty())
        spec += u": "_s + details;

    return Error(Error::WeaveFailed, origIdx, spec);
}

//-Instance Functions---------------------------------------------------------------------------------------------
//Public:
Error MultiEncoderPrivate::measureAndAccumulate(quint64& factorTotal, QList<Apportionment>& measurements, const QList<QImage>& mediums)
{
    /* This is used to determine the proportionality of all input medium images. That is, which portion of
     * the input payload will be distributed to each image. We do this by seeing much "spare" space a medium
     * has when used with a payload of 0 bytes and BPC of 1. 0 bytes means the "spare" space is actually the
     * full payload area, and using a BPC of 0 ensures that scaling effects are biased towards simply
     * increasing the real BPC instead of cause a situation where a given image will fail to fit its assigned
     * portion. This is because as BPC is increased the likelihood another byte fitting due to the nexy byte
     * boundary being reached increases, which may increase that images overall proportion. So, if we calculated
     * the apportionment at a higher BPC, but then ended up using a lower one, that corresponding amount of
     * bytes might not actually fit in the image because more space will have been lost at the lower BPC than
     * expected.
     *
     * Doing the apportionment at the lowest BPC, where such effects are the greatest, should avoid this issue.
     *
     * As an additional note, the 'total' returned here is the total capacity at 1 BPC, used for apportionment
     * only, and is not actually the total space available (i.e. at max BPC). By default we'd use an accumulate
     * function from QtConcurrent to calculate this, but that runs single-threaded at the end of the map function.
     * Instead, since we just need a simple integer sum, we can use an atomic variable that will be safely accessed
     * by any thread since the order in which it is accessed doesn't matter.
     */

    // Setup
    factorTotal = 0;

    // Measure
    std::atomic<quint64> atomicTotal;
    try{
        measurements = QtConcurrent::blockingMapped(mediums, [&, listStart = &mediums[0]](const QImage& i){
            /* We determine the original index by using the fact that QList stores
             * elements contiguously, in that we can take the difference between the
             * address of this image and the first.
             */
            qsizetype origIdx = std::distance(listStart, &i);

            if(i.isNull())
                throw MultiEncoderException(Error(Error::InvalidImage, origIdx));

            // Create apportionment, 'payload' of 0 to check for maximum space, bpc of 1 as explained above
            Measure m(mTag.size(), 0);
            auto factor = m.leftOverSpace(std::move(i.size()), 1);
            atomicTotal.fetch_add(factor, std::memory_order_relaxed); // Relaxed safe for a simple counter

            return Apportionment{
                .factor = factor,
                .origIdx = origIdx,
                .bytes = 0, // DEFAULT
                .byteWeight = 0, // DEFAULT
                .slice = {}, // DEFAULT
                .partIdx = 0 // DEFAULT
            };
        });
    } catch (MultiEncoderException& e) {
        return e.error();
    }

    factorTotal = atomicTotal;
    return {};
}

quint64 MultiEncoderPrivate::initialApportionment(quint64 factorTotal, quint64 bytesTotal, QList<Apportionment>& apportionments)
{
    /* Here we effectively do:
     * (ap.factor/factoTotal) * bytesTotal = apportionedBytes;
     *
     * However, to bypass floating point division rounding errors we multiply by the numerator first and then get the
     * truncated byte apportionment along with the remainder, and then use the remainder as a weight to assign the
     * remaining bytes later.
     */

    std::atomic<quint64> atomicRemainder = bytesTotal;

    QtConcurrent::blockingMap(apportionments, [factorTotal, bytesTotal, &atomicRemainder](Apportionment& ap){
        auto byteFactor = ap.factor * bytesTotal;
        ap.byteWeight = byteFactor % factorTotal;
        ap.bytes = byteFactor / factorTotal;
        atomicRemainder.fetch_sub(ap.bytes, std::memory_order_relaxed); // Relaxed safe for a simple counter
    });

    return atomicRemainder;
}

void MultiEncoderPrivate::finalApportionment(quint64 remainingBytes, QByteArrayView payload, QList<Apportionment>& apportionments)
{
    /* Assign leftover bytes to first N = remBytes images. We do this first discretely to try and take advantage
     * of vectorization optimizations
     */
     #pragma omp simd
     for(quint64 i = 0; i < remainingBytes; i++)
        ++apportionments[i].bytes;


    /* Assign slices in the main thread since it somewhat impractical to do multithreaded while gaining performance.
     * Due to the frequent synchronization that would be required.
     */

    quint64 offset = 0;
    for(qsizetype i = 0; i < apportionments.size(); ++i)
    {
        auto& ap = apportionments[i];
        ap.partIdx = i;
        ap.slice = payload.sliced(offset, ap.bytes);
        offset += ap.bytes;
    }
}

Error MultiEncoderPrivate::encode(QByteArrayView payload, QList<QImage>& encoded, const QList<Apportionment>& finalApportionments, const QList<QImage>& mediums)
{
    /* Here we use another atomic variable to track the offset from which the next payload slice should start. We can
     * use memory_order_relaxed here also because only the offset change needs to be atomic, but the order of everything
     * else is irrelevant.
     *
     * We also track the highest BPC used. They likely are all the same but a few might be different in edge cases.
     */

    std::atomic<Canvas::metavalue_t> bpcMax = 0;
    auto fullChecksum = Qx::Integrity::crc32(payload);

    try{
        encoded = QtConcurrent::blockingMapped(finalApportionments, [&](const Apportionment& ap){
            // Get Image
            auto idx = ap.origIdx;
            auto& image = mediums.at(idx);

            // Measurements
            Stat imageStat(image);
            Measure measurement(mTag.size(), ap.bytes);

            // Determine BPC. Likely the same amount all images, but technically can be different
            auto bpc = mBpc;
            if(bpc == 0)// Calc BPC if auto
            {
                bpc = measurement.minimumBpc(image.size());
                if(bpc == 0)
                {
                    // Check how short at max density (TODO: Make a central function for the size short string arg'ing since its reused so much)
                    quint64 max = imageStat.capacity(BPC_MAX).bytes;
                    throw MultiEncoderException(Error(Error::WontFit, idx,  u"(%1 KiB short)."_s.arg((measurement.size() - max)/1024.0, 0, 'f', 3)));
                }
            }
            else // Ensure data will fit with fixed BPC
            {
                quint64 max = imageStat.capacity(bpc).bytes;
                if(measurement.size() > max)
                    throw MultiEncoderException(Error(Error::WontFit, idx,  u"(%1 KiB short)."_s.arg((measurement.size() - max)/1024.0, 0, 'f', 3)));
            }

            // Track max BPC
            fetch_max(bpcMax, bpc);

            // Copy base image, normalize to standard format
            QImage workspace = standardizeImage(image);

            // Setup canvas, mark meta pixels, use self as reference if using relative encoding
            Canvas canvas(workspace, mPsk);
            canvas.setBpc(bpc);
            canvas.setEncoding(mEncoding);
            canvas.setReference(mEncoding == Encoder::Relative ? &workspace : nullptr);

            // Prepare for IO
            canvas.open(QIODevice::WriteOnly); // Closes upon destruction

            // Write
            MultiPartWork work(mTag, ap.slice.toByteArray(), fullChecksum, ap.partIdx, mediums.size());
            if(ArtworkError wErr = work.writeToCanvas(canvas))
                throw MultiEncoderException(fromArtworkError(wErr, idx));

            return workspace;
        });
    } catch (MultiEncoderException& e) {
        return e.error();
    }

    mBpc = bpcMax;
    return {};
}

void MultiEncoderPrivate::restoreOriginalOrder(QList<QImage>& encoded, QList<Apportionment>& apportionments)
{
    /* TODO: It would be nicer if we could avoid the initial sort, by perhaps creaing
     * a vector beside apportionments with the bias value and a reference to the equivalent
     * apportionments so that the apportionments list doesnt need to be sorted; though this
     * would increase memory use while saving for time, and we actually are mainly worried
     * about memory usage when N gets large
     */

    /* Uses cyclic permutations  sort to get back the original ordering. This mutates apportionments,
     * with it ending up in the same order as encoded, but that's OK as we don't need it anymore after
     * this function.
     *
     * https://stackoverflow.com/a/22183350/5667149
     */

    // Check all items
    for(qsizetype i = 0; i < encoded.size() - 1; ++i)
    {
        // Cycle swap until the element is at its destination
        auto dest = apportionments[i].origIdx;
        while(i != dest)
        {
            encoded.swapItemsAt(i, dest);
            apportionments.swapItemsAt(i, dest);
            dest = apportionments[i].origIdx;
        }
    }
}

/*! @endcond */

//===============================================================================================================
// MultiEncoder
//===============================================================================================================

/*!
 *  @class MultiEncoder <pxcrypt/codec/standard_encoder.h>
 *
 *  @brief The MultiEncoder class encodes a payload and identifying tag into a single image.
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
MultiEncoder::MultiEncoder() : Encoder(std::make_unique<MultiEncoderPrivate>()) {}

//-Class Functions-------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the maximum number of payload bytes that can be stored within a images of dimensions @a dims and tag of size
 *  @a tagSize when using @a bpc bits per channel.
 *
 *  @sa calculateOptimalDensity().
 */
quint64 MultiEncoder::calculateMaximumPayload(const QList<QSize>& dims, quint16 tagSize, quint8 bpc)
{
    /* Could use an atomic counter here again, but since this function is less performance critical
     * and likely sees less use, we're going to experiment with QtConcurrent's reduce functions
     */
    auto calcOneCapacity = [&](const QSize& dim){
        MultiPartWork::Measure m(tagSize, 0); // 0 size payload to check for leftover
        return m.leftOverSpace(dim, bpc);
    };
    auto sumReduce = [](quint64& sum, quint64 summand){
        sum += summand;
    };

    return QtConcurrent::blockingMappedReduced(dims, calcOneCapacity, sumReduce, 0);
}

/*!
 *  Returns the minimum number of bits per channel required to store a payload of @a payloadSize along with a tag
 *  of @a tagSize within medium images of @a dims dimensions. @c 0 is returned if the payload/tag cannot fit within
 *  those dimensions.
 *
 *  Using the lowest BPC necessary is optimal as it will produce in the least distortion per pixel and the most
 *  even spread of the encoded data, overall resulting in the most minimal visual disturbance of the original image.
 *
 *  @note With this encoder, the returned value is approximate and may differ by +/-1 given the fact that when
 *  actually encoding a payload with auto BPC, the BPC used can differ per image. Usually, it is the same for
 *  all images, but in some cases can vary +/-1 for some.
 *
 *  @sa calculateMaximumStorage().
 */
quint8 MultiEncoder::calculateOptimalDensity(const QList<QSize>& dims, quint16 tagSize, quint32 payloadSize)
{
    // TODO: Could try this in parallel with C++26's fetch_min
    // Calculate a rough estimate using the smallest image (since it will suffer from header overhead the most)

    quint64 totalArea = 0;
    quint64 smallestArea = std::numeric_limits<quint64>::max();
    QSize smallestDim;

    for(const QSize& dim : dims)
    {
        auto area = quint64(dim.width()) * quint64(dim.height());
        totalArea += area;
        if(area < smallestArea)
        {
            smallestArea = area;
            smallestDim = dim;
        }
    }

    quint32 estByteApportionment = std::round((double(smallestArea)/totalArea) * payloadSize);
    PxCryptPrivate::MultiPartWork::Measure m(tagSize, estByteApportionment);
    return m.minimumBpc(smallestDim);
}

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the encoding tag.
 *
 *  @sa setTag().
 */
QString MultiEncoder::tag() const { Q_D(const MultiEncoder); return d->mTag; }

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
void MultiEncoder::setTag(const QByteArray& tag) { Q_D(MultiEncoder); d->mTag = tag; }

/*!
 *  NOTE: Note how the order of the parts wont necessarily be the same as mediums
 *
 *  @sa StandardDecoder::decode().
 */
/*!
 *  Encodes @a payload within the medium image @a medium and stores the result in @a encoded, then
 *  returns an error status.
 *
 *  Encodes @a payload within the images from @a mediums, distributed proportionally according to their
 *  dimmensions. That is, the bytes of the original payload are split into chunks and spread across all
 *  medium images.
 *
 *  The encoding is carried out in accordance with the current configuration of the encoder.
 *
 *  If the current BPC of the encoder is @c 0, the best BPC for each medium is determined independently
 *  based on the size of @a payload. Usually the ideal BPC is the same for all mediums, but there are some
 *  edge cases where some might differ from the rest by +/- @c 1. The encoders BPC will be set to the highest
 *  BPC used in this case.
 *
 *  The encoded images will always use the format `QImage::Format_ARGB32` or `QImage::Format_RGB32`
 *  (depending on if the original has an alpha channel) regardless of the format of the original medium.
 *
 *  @note The encoded images are always returned in the same order as the input mediums, though this order
 *  is not necessarlly the same as the order in which @a payload was split up. This is a non-issue however
 *  as the multi-part decoder is able to determine the correct order of its input as long as all parts
 *  are present.
 *
 *  @warning
 *  @parblock
 *  The images produced by this function must never undergo a reduction in fidelity in order to remain
 *  decodable. In other words, they must be stored in a format that uses 8 bits per color channel and holds at least
 *  the RGB channels, else the encoded data be corrupted. No data is encoded within alpha channels so they can
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
Error MultiEncoder::encode(QList<QImage>& encoded, QByteArrayView payload, const QList<QImage>& mediums)
{
    /* NOTE: This doesn't use the PSK to generate new seeds for per-image PRNGs, which would be nice
     * for added scrambling, but currently impractical since we want to support loading the images
     * back in an arbitrary order. This could be achieved though if we used a mechanism at an
     * abstraction level above the device. Leave the devices RNG alone as standard, but fuel a per-image
     * PRNG and use that to shuffle the byte order around, maybe even as simple as RNG between 0 and 1 and
     * use 0 to add to front of Canvas and 1 to back until complete. Then just write the header bytes for this
     * artwork type in order, leaving only the payload double scrambled. This way the header could still tell us
     * which artwork goes in what order, which would let us match the correct per-image PRNG and the decode.
     */
    Q_D(MultiEncoder);

    // Clear return buffer
    encoded.clear();

    // Ensure data was provided
    if(payload.isEmpty())
        return Error(Error::MissingPayload);

    // Ensure images were provided
    if(mediums.isEmpty())
        return Error(Error::MissingMediums);

    // Ensure bits-per-channel is valid (NOTE: optionally could clamp instead)
    if(d->mBpc > BPC_MAX)
        return Error(Error::InvalidBpc);

    // Measure proportions
    quint64 factorTotal;
    QList<Apportionment> apportionments;
    if(auto err = d->measureAndAccumulate(factorTotal, apportionments, mediums); err.isValid())
        return err;

    // Perform initial apportionment
    quint64 remBytes = d->initialApportionment(factorTotal, payload.size(), apportionments);
    Q_ASSERT(remBytes < static_cast<quint64>(mediums.size()));

    /* Sort by bias; Technically, we only need the top N to be sorted where N = remBytes, but
     * that value could be a significant portion of the range and in that case it's known that
     * implementations of partial_sort may actually be slower than a full sort so we just
     * sort the whole thing.
     */
    std::sort(std::execution::par, apportionments.begin(), apportionments.end(), [](const Apportionment& a, const Apportionment& b){
        return a.byteWeight > b.byteWeight;
    });

    // Assign remaining bytes and slices
    d->finalApportionment(remBytes, payload, apportionments);

    // Encode
    QList<QImage> tempEncoded;
    if(auto err = d->encode(payload, tempEncoded, apportionments, mediums); err.isValid())
        return err;

    // Reorder to original input order
    d->restoreOriginalOrder(tempEncoded, apportionments);

    encoded = tempEncoded;

    return Error();
}

//===============================================================================================================
// MultiEncoder::Error
//===============================================================================================================

/*!
 *  @class MultiEncoder::Error <pxcrypt/codec/multi_encoder.h>
 *
 *  @brief The MultiEncoder::Error class is used to report errors during multi-image encoding.
 *
 *  @sa MultiEncoder and MultiDecoder::Error.
 */

//-Class Enums-----------------------------------------------------------------------------------------------------
/*!
 *  @enum MultiEncoder::Error::Type
 *
 *  This enum specifies the type of encoding error that occurred.
 *
 *  @var MultiEncoder::Error::Type MultiEncoder::Error::NoError
 *  No error occurred.
 *
 *  @var MultiEncoder::Error::Type MultiEncoder::Error::MissingPayload
 *  No payload data was provided.
 *
 *  @var MultiEncoder::Error::Type MultiEncoder::Error::MissingMediums
 *  No medium images were provided.
 *
 *  @var MultiEncoder::Error::Type MultiEncoder::Error::InvalidImage
 *  A medium was invalid.
 *
 *  @var MultiEncoder::Error::Type MultiEncoder::Error::WontFit
 *  A medium's dimensions were not large enough to fit the payload.
 *
 *  @var MultiEncoder::Error::Type MultiEncoder::Error::InvalidBpc
 *  The BPC value was not between 1 and 7.
 *
 *  @var MultiEncoder::Error::Type MultiEncoder::Error::WeaveFailed
 *  An unexpected error occurred while weaving data into a medium.
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
Error::Error(Type t, qsizetype idx, const QString& s) :
    mType(t),
    mSpecific(s),
    mImageIndex(idx)
{}

//-Instance Functions-------------------------------------------------------------
//Private:
quint32 Error::deriveValue() const { return mType; }
QString Error::derivePrimary() const { return PREFIX_STRING + ' ' + ERR_STRINGS.value(mType); }
QString Error::deriveSecondary() const { return mSpecific; }

//Public:
/*!
 *  Returns @c true if the error's type is one other than NoError; otherwise, returns @c false.
 *
 *  @sa Type.
 */
bool MultiEncoder::Error::isValid() const { return mType != NoError; }

/*!
 *  Returns the type of encoding error.
 *
 *  @sa errorString().
 */
Error::Type MultiEncoder::Error::type() const { return mType; }

/*!
 *  Returns the index of the image that caused the error, or @c -1 if the error is not image specific.
 */
qsizetype MultiEncoder::Error::imageIndex() const { return mImageIndex; }

/*!
 *  Returns instance specific error information, if any.
 *
 *  @sa errorString().
 */
QString MultiEncoder::Error::specific() const { return mSpecific; }

/*!
 *  Returns the string representation of the error.
 *
 *  @sa type().
 */
QString MultiEncoder::Error::errorString() const
{
    QString str = derivePrimary();
    if(mImageIndex >= 0)
        str += '[' + QString::number(mImageIndex) + ']';
    if(!mSpecific.isEmpty())
        str += ' ' + mSpecific;
    return derivePrimary() +  (mSpecific.isEmpty() ? "" : ' ' + mSpecific);
}
}
