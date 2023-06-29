// Unit Includes
#include "pxcrypt/encoder.h"

// Project Includes
#include "encdec.h"
#include "tools/px_access_write.h"
#include "tools/px_weaver.h"
#include "tools/bit_chunker.h"

// Qx Includes
#include <qx/core/qx-bitarray.h>
#include <qx/core/qx-integrity.h>

namespace PxCrypt
{

namespace // Implementation details
{
    EncStrat underlyingStrategy(Encoder::Encoding enc)
    {
        switch(enc)
        {
            case Encoder::Absolute:
                return EncStrat::Direct;
            case Encoder::Relative:
                return EncStrat::Displaced;
            default:
                qCritical("Unhandled encoding!");
        };

        return static_cast<EncStrat>(EncStrat::Direct); // Never reached, used to silence control path warning
    }
}

//===============================================================================================================
// Encoder
//===============================================================================================================

/*!
 *  @class Encoder <pxcrypt/encoder.h>
 *
 *  @brief The Encoder class provides serialization of binary data to portions of an arbitrary image's color
 *  channels.
 */

//-Class Enums-----------------------------------------------------------------------------------------------------
/*!
 *  @enum Encoder::Encoding
 *
 *  This enum specifies the encoding strategy utilized when encoding/decoding data, which
 *  affects several aspects of the resultant image.
 *
 *  @par Relative
 *  @parblock
 *  This is the default encoding strategy.
 *
 *  The input data is broken up into 1-7 bit-wide frames in according to the value selected for
 *  bits per channel which are then woven into existing pixel data with each frame being mapped to
 *  one channel of a given pixel. This is accomplished by applying an offset to the original color
 *  channel value that matches the magnitude of the frame. The frame is subtracted from the original
 *  value if it is greater than @c 127; otherwise, the frame is added to the original value.
 *
 *  Because the data is not stored directly in the image, but rather as the difference between the
 *  original image and the encrypted image, the is no way to know what the offset for each channel
 *  was without a point of reference, and thus the original medium image is required in order to
 *  decode the encoded data. Payloads are more securely protected using this method.
 *
 *  The effect each frame has on a given channel is directly proportional to its magnitude, with
 *  higher value frames causing greater distortion in the original image. The best case scenario
 *  occurs when the frame has a value of zero, while the worst is when the frame contains the maximum
 *  value allowed by the BPC setting. Overall this means that the choice of medium has no effect on
 *  the amount of distortion and that the degree of distortion at a given BPC is influenced entirely
 *  by the input payload when using this strategy, with ideal data consisting largely of low bits.
 *
 *  Best Case) 0
 *
 *  Worst Case) 2<SUP>N</SUP> - 1 (where N = bits per channel)
 *  @endparblock
 *
 *  @par Absolute
 *  @parblock
 *  Input data is broken up into frames in the same manner as the Relative strategy, but are simply
 *  inserted directly into a pixel's channel data by replacing the lowest bits up to amount allowed
 *  by the BPC value. This allows the convenience of not requiring the original medium to decode
 *  the encrypted data, but at the cost of security since the output image relies entirely on the
 *  strength of the pre shared key for protection.
 *
 *  With this strategy the potential distortion caused by each frame depends on both the input data
 *  and the selected medium, as the degree of change is dictated by how different the frame is compared
 *  to the original bits that are being replaced. The best case scenario occurs when the frames data
 *  matches the original exactly, while the worst is when it is completely different. This leads to the
 *  same range of potential impact on the original image as with the Relative strategy, but in theory
 *  means that this method can out perform said strategy on average when the variance of the payload
 *  closes matches the variance of the image's color data
 *
 *  Best Case) 0
 *
 *  Worst Case) 2<SUP>N</SUP> - 1 (where N = bits per channel)
 *  @endparblock
 *
 *  @var Encoder::Encoding Encoder::Relative
 *  Requires the original medium in order to decode the encrypted data.
 *
 *  @var Encoder::Encoding Encoder::Absolute
 *  Does not require the original medium in order to decode the encrypted data.
 */

//-Constructor-----------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an encoder with default settings:
 *  - BPC of 1
 *  - Empty pre-shared key
 *  - Absolute encoding
 *  - Empty tag
 */
Encoder::Encoder() :
    mBpc(1),
    mPsk(),
    mEncoding(Absolute),
    mTag(),
    mError()
{}

//-Class Functions-------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the maximum number of bytes that can be stored within an image of dimensions @a dim and tag of size
 *  @a tagSize when using @a bpc bits per channel.
 *
 *  @sa calculateOptimalDensity().
 */
quint64 Encoder::calculateMaximumStorage(const QSize& dim, quint16 tagSize, quint8 bpc)
{
    /* NOTE: Both decode.h and encode.h need the internal version of this function
     * so using a wrapper for the public version prevents including decode.h from
     * automatically including encode.h
     */
    return calcMaxPayloadBytes(dim, tagSize, bpc);
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
quint8 Encoder::calculateOptimalDensity(const QSize& dim, quint16 tagSize, quint32 payloadSize)
{
    // Returns the minimum BPC required to store `payload`, or 0 if the
    // payload will not fit within `dim`
    if(dim.width() == 0 || dim.height() == 0)
        return 0;

    double bits = (payloadSize + tagSize + HEADER_BYTES) * 8.0;
    double chunks = (dim.width() * dim.height() - META_PIXELS) * 3.0;
    double bpc = std::ceil(bits/chunks);

    return bpc < 8 ? bpc : 0;
}

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if encoding failed.
 *
 *  Use error() to retrieve the cause. Calls to encode() are ignored if the encoder's error state is set.
 *
 *  @sa reset().
 */
bool Encoder::hasError() const { return mError.isValid(); }

/*!
 *  Returns the error encountered while encoding, if any; otherwise, returns an invalid error.
 *
 *  @sa hasError() and reset().
 */
Qx::GenericError Encoder::error() const { return mError; }

/*!
 *  Resets the error status of the encoder.
 *
 *  @sa error().
 */
void Encoder::reset() { mError = {}; }

/*!
 *  Returns the number of bits-per-channel the encoder is configured to use.
 *
 *  @sa setBpc().
 */
quint8 Encoder::bpc() const { return mBpc; }

/*!
 *  Returns the key the encoder is configured to use for data scrambling.
 *
 *  @sa setPresharedKey().
 */
QByteArray Encoder::presharedKey() const { return mPsk; }

/*!
 *  Returns the encoding strategy the encoder is configured to use.
 *
 *  @sa setEncoding().
 */
Encoder::Encoding Encoder::encoding() const { return mEncoding; }

/*!
 *  Returns the encoding tag.
 *
 *  @sa setTag().
 */
QString Encoder::tag() const { return mTag; }

/*!
 *  Sets the number of bits-per-channel the encoder is configured to use to @a bpc.
 *
 *  This settings directly determines the number of bits woven into each color channel of a pixel and affects
 *  total encoding capacity. A BPC of 0 will instruct the encoder to determine the optimal BPC count automatically.
 *
 *  @sa bpc().
 */
void Encoder::setBpc(quint8 bpc) { mBpc = bpc; }

/*!
 *  Sets key used for scrambling the encoding sequence to @a key.
 *
 *  This key will be required in order to decode the resultant encoded data. An empty string results in the use
 *  of a known/default encoding sequence.
 *
 *  @sa presharedKey().
 */
void Encoder::setPresharedKey(const QByteArray& key) { mPsk = key; }

/*!
 *  Sets the encoding strategy to use to @a enc.
 *
 *  @sa encoding() and Encoding.
 */
void Encoder::setEncoding(Encoding enc) { mEncoding = enc; }

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
void Encoder::setTag(const QString tag) { mTag = tag; }

/*!
 *  Encodes @a payload within the medium image @a medium and returns it.
 *
 *  The encoding is carried out in accordance with the current configuration of the encoder.
 *
 *  If the current BPC of the encoder is @c 0, the best BPC for the given medium and payload will be determined
 *  automatically. Then, the encoder's BPC is set to that ideal value.
 *
 *  If encoding fails, a null image will be returned. Use error() to determine the cause. Further attempts
 *  to use encode() will be ignored until the error state is cleared via reset().
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
 *  @sa Decoder::decode().
 */
QImage Encoder::encode(QByteArrayView payload, const QImage& medium)
{
    if(hasError())
        return QImage();

    /* NOTE: Because the data chunked by BitChunker won't always align to a byte boundary at the end
     * (i.e. the final chunk of a byte sequence might not actually contain enough bits to reach the
     * bpc size), all data encoded by this function must be done so using a contiguous array so that
     * no incomplete chunk is woven into a pixel (this would effectively cause null padding bits to be
     * present in the encoded result that the decoder wouldn't be able to detect).
     *
     * An alternative is to change PxWeaver to have a data buffer of its own and change it so that it can
     * be told it's received an unfinished chunk and wait till it receives the rest to weave it; this is
     * undesirably clunky however.
     *
     * Instead, for now the increased memory usage of copying the payload array into a final array will
     * be considered acceptable given the relatively small amounts of data that can be encoded with
     * this format. A good candidate for remedying this is std::views::concat which is slated to be
     * added in C++26 (allows creating a view of an arbitrary number of heterogeneous containers that
     * can be iterated over sequentially as long as they have the same value type). With this the header
     * and tag portions can be created as separate arrays and then they and the original payload view can
     * be iterated over as they are without a copy.
     *
     * https://github.com/cplusplus/papers/issues/1204
     */

    // Ensure data was provided
    if(payload.isEmpty())
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_ENCODING_FAILED, ERR_NO_DATA);
        return QImage();
    }

    // Ensure bits-per-channel is valid (TODO: optionally could clamp instead)
    if(mBpc > 7)
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_ENCODING_FAILED, ERR_INVALID_BPC);
        return QImage();
    }

    // Ensure image is valid
    if(medium.isNull())
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_ENCODING_FAILED, ERR_INVALID_IMAGE);
        return QImage();
    }

    // Convert tag to UTF-8 and cap to storage cap
    QByteArray tagData = mTag.toUtf8();
    if(tagData.size() > std::numeric_limits<quint16>::max())
        tagData.resize(std::numeric_limits<quint16>::max());

    // Determine BPC if auto
    if(mBpc == 0)
    {
        mBpc = calculateOptimalDensity(medium.size(), tagData.size(), payload.size());
        if(mBpc == 0)
        {
            // Check how short at max density
            quint64 max = calcMaxPayloadBytes(medium.size(), tagData.size(), 7);
            mError =  Qx::GenericError(Qx::GenericError::Critical, ERR_ENCODING_FAILED, ERR_WONT_FIT.arg((payload.size() - max)/1024.0, 0, 'f', 2));
            return QImage();
        }
    }

    // Ensure data will fit
    quint64 maxStorage = calcMaxPayloadBytes(medium.size(), tagData.size(), mBpc);
    if(static_cast<quint64>(payload.size()) > maxStorage)
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_ENCODING_FAILED, ERR_WONT_FIT.arg((payload.size() - maxStorage)/1024.0, 0, 'f', 2));
        return QImage();
    }

    // Create full data array
    QByteArray fullData;
    fullData.reserve(HEADER_BYTES + tagData.size() + payload.size());

    // Add header to array
    QDataStream hs(&fullData, QIODeviceBase::WriteOnly);
    hs.writeRawData(MAGIC_NUM, MAGIC_SIZE);
    hs << Qx::Integrity::crc32(payload);
    hs << static_cast<quint16>(tagData.size());
    hs << static_cast<quint32>(payload.size());

    // Add tag and payload to array
    fullData.append(tagData);
    fullData.append(payload);

    // Copy base image, normalize to standard format
    QImage encoded = standardizeImage(medium);

    // Setup pixel access, marks meta data on image
    PxAccessWrite pAccess(&encoded, !mPsk.isEmpty() ? mPsk : DEFAULT_SEED, mBpc, underlyingStrategy(mEncoding));

    // Prepare pixel weaver
    PxWeaver pWeaver(&pAccess);

    // Encode full array
    BitChunker bChunker(fullData, mBpc);
    while(bChunker.hasNext())
        pWeaver.weave(bChunker.next());

    // Flusher weaver
    pWeaver.flush();

    return encoded;
}

}
