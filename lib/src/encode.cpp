// Unit Includes
#include "pxcrypt/encode.h"

// Project Includes
#include "pxcrypt/encdec.h"
#include "encdec_p.h"
#include "tools/px_access_write.h"
#include "tools/px_weaver.h"
#include "tools/bit_chunker.h"

// Qx Includes
#include <qx/core/qx-bitarray.h>
#include <qx/core/qx-integrity.h>

/*!
 *  @file encode.h
 *
 *  @brief The encode header file provides facilities relevant to encoding data within an image.
 */

namespace PxCrypt
{

namespace
{
    //-Unit Private Variables ---------------------------------------------------------------------------------------------

    // Errors
    const QString ERR_ENCODING_FAILED = QSL("Encoding failed.");
    const QString ERR_NO_DATA = QSL("No data was provided");
    const QString ERR_INVALID_IMAGE = QSL("The medium is invalid.");
    const QString ERR_WONT_FIT = QSL("The medium's dimmensions are not large enough to fit the payload (%1 KiB short).");
    const QString ERR_INVALID_BPC = QSL("Bits-per-channel must be between 1 and 7.");
}

//-Namespace Types-------------------------------------------------------------------------------------------------

//===============================================================================================================
// EncodeSettings
//===============================================================================================================

/*!
 *  @struct EncodeSettings <pxcrypt/encode.h>
 *
 *  @brief The EncodeSettings struct holds tuning parameters that affect how data is encoded into an image.
 *
 *  @sa encode().
 */

/*!
 *  @var quint8 EncodeSettings::bpc
 *
 *  The number of bits woven into each color channel of a pixel.
 *
 *  Affects the total encoding capacity. Settings a BPC of 0 will instruct the encoder to determine the
 *  optimal BPC count automatically.
 */

/*!
 *  @var QStringView EncodeSettings::psk
 *
 *  The key used for scrambling the encoding sequence.
 *
 *  Required in order to decode the resultant encoded data. An
 *  empty string results in the use of a known/default encoding sequence.
 */

/*!
 *  @var EncType EncodeSettings::type
 *
 *  The encoding strategy to use. Needs to be known when decoding the resultant data.
 */

//-Namespace Functions-------------------------------------------------------------------------------------------------
/*!
 *  Returns the maximum number of bytes that can be stored within an image of dimensions @a dim and tag of size
 *  @a tagSize when using @a bpc bits per channel.
 *
 *  @sa calculateOptimalDensity().
 */
quint64 calculateMaximumStorage(const QSize& dim, quint16 tagSize, quint8 bpc)
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
quint8 calculateOptimalDensity(const QSize& dim, quint16 tagSize, quint32 payloadSize)
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

/*!
 *  Encodes a payload within a given medium image.
 *
 *  @param[out] enc The resultant image with the encoded payload.
 *  @param[in] medium The image to encode the data within
 *  @param[in] tag An optional identifier string to couple with the encoded payload
 *  @param[in] payload The data to encode
 *  @param[in] set Tuneables for the encoding technique
 *  @return An error object that describes cause of failure if encoding fails.
 *
 *  @a enc will always use the format `QImage::Format_ARGB32` or `QImage::Format_RGB32`
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
 *  @sa decode(), EncodeSettings.
 */
Qx::GenericError encode(QImage& enc, const QImage& medium, QStringView tag, QByteArrayView payload, EncodeSettings& set)
{
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

    // BPC of 0 means "auto"

    // Clear buffer
    enc = QImage();

    // Ensure data was provided
    if(payload.isEmpty())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_ENCODING_FAILED, ERR_NO_DATA);

    // Ensure bits-per-channel is valid (TODO: optionally could clamp instead)
    if(set.bpc > 7)
        return Qx::GenericError(Qx::GenericError::Critical, ERR_ENCODING_FAILED, ERR_INVALID_BPC);

    // Ensure image is valid
    if(medium.isNull())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_ENCODING_FAILED, ERR_INVALID_IMAGE);

    // Convert tag to UTF-8 and cap to storage cap
    QByteArray tagData = tag.toUtf8();
    if(tagData.size() > std::numeric_limits<quint16>::max())
        tagData.resize(std::numeric_limits<quint16>::max());

    // Determine BPC if auto
    if(set.bpc == 0)
    {
        set.bpc = calculateOptimalDensity(medium.size(), tagData.size(), payload.size());
        if(set.bpc == 0)
        {
            // Check how short at max density
            quint64 max = calcMaxPayloadBytes(medium.size(), tagData.size(), 7);
            return Qx::GenericError(Qx::GenericError::Critical, ERR_ENCODING_FAILED, ERR_WONT_FIT.arg((payload.size() - max)/1024.0, 0, 'f', 2));
        }
    }

    // Ensure data will fit
    quint64 maxStorage = calcMaxPayloadBytes(medium.size(), tagData.size(), set.bpc);
    if(static_cast<quint64>(payload.size()) > maxStorage)
        return Qx::GenericError(Qx::GenericError::Critical, ERR_ENCODING_FAILED, ERR_WONT_FIT.arg((payload.size() - maxStorage)/1024.0, 0, 'f', 2));

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
    enc = standardizeImage(medium);

    // Setup pixel access, marks meta data on image
    PxAccessWrite pAccess(&enc, !set.psk.isEmpty() ? set.psk : DEFAULT_SEED, set.bpc, set.type);

    // Prepare pixel weaver
    PxWeaver pWeaver(&pAccess);

    // Encode full array
    BitChunker bChunker(fullData, set.bpc);
    while(bChunker.hasNext())
        pWeaver.weave(bChunker.next());

    // Flusher weaver
    pWeaver.flush();

    return Qx::GenericError();
}

}
