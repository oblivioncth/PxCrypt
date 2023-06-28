// Unit Includes
#include "pxcrypt/decoder.h"

// Project Includes
#include "encdec.h"
#include "tools/px_access_read.h"
#include "tools/px_skimmer.h"
#include "tools/byte_compositer.h"

// Qx Includes
#include <qx/core/qx-bitarray.h>
#include <qx/core/qx-integrity.h>

// Magic Enum Include
#include <magic_enum.hpp>

namespace PxCrypt
{

//===============================================================================================================
// Decoder
//===============================================================================================================

/*!
 *  @class Decoder <pxcrypt/decoder.h>
 *
 *  @brief The Decoder class provides the means to skim and decrypt data from the color channels of an
 *  encoded image.
 */

//-Constructor-----------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Constructs an encoder with an empty pre-shared key.
 */
Decoder::Decoder() :
    mPsk(),
    mTag(),
    mError()
{}

//-Class Functions-------------------------------------------------------------------------------------------------
//Private:
bool Decoder::canFitHeader(const QSize& dim, quint8 bpc)
{
    int bits = (dim.width() * dim.height() - META_PIXELS) * 3 * bpc;
    return bits >= HEADER_BYTES * 8;
}

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns @c true if decoding failed.
 *
 *  Use error() to retrieve the cause. Calls to decode() are ignored if the decoder's error state is set.
 *
 *  @sa reset().
 */
bool Decoder::hasError() const { return mError.isValid(); }

/*!
 *  Returns the error encountered while decoding, if any; otherwise, returns an invalid error.
 *
 *  @sa hasError() and reset().
 */
Qx::GenericError Decoder::error() const { return mError; }

/*!
 *  Resets the error status of the decoder.
 *
 *  @sa error().
 */
void Decoder::reset() { mError = {}; }

/*!
 *  Returns the tag of the last successfully decoded payload.
 *
 *  @sa Encoder::setTag().
 */
QString Decoder::tag() const { return mTag; };

/*!
 *  Returns the key the decoder is configured to use for descrambling data.
 *
 *  @sa setPresharedKey().
 */
QByteArray Decoder::presharedKey() const { return mPsk; }

/*!
 *  Sets key used for descrambling the encoding data to @a key.
 *
 *  This key must be the same that was used when the data was encoded. An empty string results in the use
 *  of a known/default decoding sequence.
 *
 *  @sa presharedKey().
 */
void Decoder::setPresharedKey(const QByteArray& key) { mPsk = key; }


/*!
 *  Retrieves data from the encoded PxCrypt image @a encoded and returns it as a byte array.
 *
 *  The original medium image @a medium is used as a reference for images with encodings that require it, but
 *  is otherwise ignored. The encoded data is descrambled using the currently set pre-shared key.
 *
 *  If encoding fails, a null byte array will be returned. Use error() to determine the cause. Further attempts
 *  to use decode() will be ignored until the error state is cleared via reset(). After an image is successfully
 *  decoded the tag of the encoded data is made available via tag().
 *
 *  @sa Encoder::encode().
 */
QByteArray Decoder::decode(const QImage& encoded, const QImage& medium)
{
    if(hasError())
        return QByteArray();

    // Ensure encoded image is valid
    if(encoded.isNull())
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_INVALID_SOURCE);
        return QByteArray();
    }

    // Ensure image meets bare minimum space for meta pixels
    if(encoded.size().width() * encoded.size().height() < META_PIXELS)
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_NOT_LARGE_ENOUGH);
        return QByteArray();
    }

    // Ensure standard pixel format
    QImage encStd = standardizeImage(encoded);

    // Prepare pixel access, automatically reads meta pixels
    PxAccessRead pAccess(&encStd, !mPsk.isEmpty() ? mPsk : DEFAULT_SEED);

    // Ensure BPC is valid
    if(pAccess.bpc() < 1 || pAccess.bpc() > 7)
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_INVALID_META);
        return QByteArray();
    }

    // Ensure strat is valid
    if(!magic_enum::enum_contains(pAccess.strat()))
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_INVALID_META);
        return QByteArray();
    }

    // Ensure at least header can fit
    if(!canFitHeader(encStd.size(), pAccess.bpc()))
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_NOT_LARGE_ENOUGH);
        return QByteArray();
    }

    // Reserve space for header
    QByteArray decoded;
    decoded.reserve(HEADER_BYTES);

    // Ensure medium image is valid if applicable
    QImage mediumStd;
    if(pAccess.strat() == EncStrat::Displaced)
    {
        if(medium.isNull())
        {
            mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_MISSING_MEDIUM);
            return QByteArray();
        }

        if(medium.size() != encStd.size())
        {
            mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_DIMMENSION_MISTMATCH);
            return QByteArray();
        }

        mediumStd = standardizeImage(medium);
    }

    // Prepare pixel skimmer and byte compositer
    PxSkimmer pSkimmer(&pAccess, &mediumStd);
    ByteCompositer bCompositer(&decoded, pAccess.bpc());

    // Read header
    while(decoded.size() != HEADER_BYTES)
        bCompositer.composite(pSkimmer.next());

    // Extract header components
    QDataStream hs(decoded);
    QByteArray hMagic(MAGIC_NUM.size(), Qt::Uninitialized);
    quint32 hPayloadChecksum;
    quint16 hTagSize;
    quint32 hPayloadSize;

    hs.readRawData(hMagic.data(), hMagic.size());
    hs >> hPayloadChecksum;
    hs >> hTagSize;
    hs >> hPayloadSize;

    // Ensure header is valid
    if(hMagic != MAGIC_NUM)
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_INVALID_HEADER);
        return QByteArray();
    }

    quint64 maxStorage = calcMaxPayloadBytes(encStd.size(), hTagSize, pAccess.bpc());
    if(hPayloadSize > maxStorage)
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_NOT_LARGE_ENOUGH);
        return QByteArray();
    }

    // Clear header, prep output for tag
    decoded.clear();
    decoded.reserve(hTagSize);

    // Read tag
    while(decoded.size() != hTagSize)
        bCompositer.composite(pSkimmer.next());
    QString tag = QString::fromUtf8(decoded.constData());

    // Clear tag, prep output for payloads
    decoded.clear();
    decoded.reserve(hPayloadSize);

    // Read payload
    while(decoded.size() != hPayloadSize && !pSkimmer.isAtLimit())
        bCompositer.composite(pSkimmer.next());

    if(decoded.size() != hPayloadSize)
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_UNEXPECTED_END);
        return QByteArray();
    }

    // Validate payload
    quint32 checksum = Qx::Integrity::crc32(decoded);
    if(checksum != hPayloadChecksum)
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_CHECKSUM_MISMATCH);
        return QByteArray();
    }

    mTag = tag; // Only store tags from successfully decoded images
    return decoded;
}

}
