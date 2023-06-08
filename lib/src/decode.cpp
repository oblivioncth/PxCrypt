// Unit Includes
#include "pxcrypt/decode.h"

// Project Includes
#include "pxcrypt/encdec.h"
#include "encdec_p.h"
#include "tools/px_skimmer.h"
#include "tools/byte_compositer.h"

// Qx Includes
#include <qx/core/qx-bitarray.h>
#include <qx/core/qx-integrity.h>

/*!
 *  @file decode.h
 *
 *  @brief The decode header file provides facilities relevant to decoding data stored within an image.
 */

namespace PxCrypt
{

namespace
{
    //-Unit Private Variables ---------------------------------------------------------------------------------------------

    // Errors
    const QString ERR_DECODING_FAILED = QSL("Decoding failed.");
    const QString ERR_INVALID_SOURCE = QSL("The encoded image is invalid.");
    const QString ERR_MISSING_MEDIUM = QSL("The required medium image is invalid.");
    const QString ERR_DIMMENSION_MISTMATCH = QSL("The required medium image has different dimmensions than the encoded image.");
    const QString ERR_NOT_LARGE_ENOUGH = QSL("The provided image is not large enough to be an encoded image.");
    const QString ERR_INVALID_HEADER = QSL("The provided image is not encoded, the wrong encoding type was specified,"
                                           " or the password/medium are incorrect.");
    const QString ERR_LENGTH_MISMATCH = QSL("The encoded image's header indicates it contains more data than possible.");
    const QString ERR_UNEXPECTED_END = QSL("All pixels were skimmed before the expected payload size was reached.");
    const QString ERR_CHECKSUM_MISMATCH = QSL("The payload's checksum did not match the expected value.");

    //-Unit Private Functions ---------------------------------------------------------------------------------------------
    bool canFitHeader(const QSize& dim, quint8 bpc)
    {
        int bits = (dim.width() * dim.height() - 1) * 3 * bpc; // -1 For bpp indicator
        return bits >= HEADER_BYTES * 8;
    }
}

//-Namespace Types-------------------------------------------------------------------------------------------------

//===============================================================================================================
// DecodeSettings
//===============================================================================================================

/*!
 *  @struct DecodeSettings <pxcrypt/decode.h>
 *
 *  @brief The DecodeSettings struct holds tuning parameters that are required to properly decode
 *  data from an image.
 */

/*!
 *  @var QByteArray DecodeSettings::psk
 *
 *  The key that was used when encoding the original data, if any.
 */

/*!
 *  @var EncType DecodeSettings::type
 *
 *  The encoding strategy that was used when encoding the original data.
 */

//-Namespace Functions-------------------------------------------------------------------------------------------------

/*!
 *  Retrieves data from a PxCrypt image.
 *
 *  @param[out] dec The original payload.
 *  @param[out] tag The image's/payload's tag
 *  @param[in] enc The image with the encoded data
 *  @param[in] set Tuneables for properly decoding the input image
 *  @param[in] medium The original medium used when encoding the payload. Ignored for encoding types other than EncType::Relative
 *  @return An error object that describes cause of failure if encoding fails.
 *
 *  @sa encode(), DecodeSettings.
 */
Qx::GenericError decode(QByteArray& dec, QString& tag, const QImage& enc, DecodeSettings set, const QImage& medium)
{
    // Clear buffer
    dec.clear();

    // Ensure encoded image is valid
    if(enc.isNull())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_INVALID_SOURCE);

    // Ensure medium image is valid if applicable
    if(set.type == EncType::Relative)
    {
        if(medium.isNull())
            return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_MISSING_MEDIUM);

        if(medium.size() != enc.size())
            return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_DIMMENSION_MISTMATCH);
    }

    // Ensure standard pixel format
    QImage encStd = enc; // Because of Qt's CoW system this occurs almost no penalty if the format is already correct
    if(encStd.format() != QImage::Format_RGBA8888)
        encStd.convertTo(QImage::Format_RGBA8888);

    QImage mediumStd = medium;
    if(!mediumStd.isNull() && mediumStd.format() != QImage::Format_RGBA8888)
        mediumStd.convertTo(QImage::Format_RGBA8888);

    // Check BPC
    Qx::BitArray bpcBits(3);

    QRgb first = enc.pixel(0, 0);
    bpcBits.setBit(0, qRed(first) & 0x01);
    bpcBits.setBit(1, qGreen(first) & 0x01);
    bpcBits.setBit(2, qBlue(first) & 0x01);
    quint8 bpc = bpcBits.toInteger<quint8>();

    // Ensure at least header can fit
    if(!canFitHeader(encStd.size(), bpc))
        return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_NOT_LARGE_ENOUGH);

    // Reserve space for header
    dec.reserve(HEADER_BYTES);

    // Prepare pixel skimmer and byte compositer
    PxSkimmer pSkimmer(&encStd, &mediumStd, set.psk.isEmpty() ? set.psk : DEFAULT_SEED, bpc, set.type);
    ByteCompositer bCompositer(&dec, bpc);

    // Read header
    while(dec.size() != HEADER_BYTES)
        bCompositer.composite(pSkimmer.next());

    // Extract header components
    QDataStream hs(dec);
    QByteArray hMagic(MAGIC_NUM.size(), Qt::Uninitialized);
    EncType hType;
    quint32 hPayloadChecksum;
    quint16 hTagSize;
    quint32 hPayloadSize;

    hs.readRawData(hMagic.data(), hMagic.size());
    hs >> hType;
    hs >> hPayloadChecksum;
    hs >> hTagSize;
    hs >> hPayloadSize;

    // Ensure header is valid
    if(hMagic != MAGIC_NUM)
        return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_INVALID_HEADER);

    if(hType != set.type)
    {
        /* NOTE: This should be impossible given the data would be garbage with
         * the wrong encoding type, hence why this only causes a warning;
         * however, potential future data types may require re-evaluating this check.
         */
        qWarning("mismatched encoding type!");
    }

    quint64 maxStorage = calcMaxPayloadSize(encStd.size(), hTagSize, bpc);
    if(hPayloadSize > maxStorage)
        return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_NOT_LARGE_ENOUGH);

    // Clear header, prep output for tag
    dec.clear();
    dec.reserve(hTagSize);

    // Read tag
    while(dec.size() != hTagSize)
        bCompositer.composite(pSkimmer.next());
    tag = QString::fromUtf8(dec.constData());

    // Clear tag, prep output for payloads
    dec.clear();
    dec.reserve(hPayloadSize);

    // Read payload
    while(dec.size() != hPayloadSize && !pSkimmer.isAtLimit())
        bCompositer.composite(pSkimmer.next());

    if(dec.size() != hPayloadSize)
        return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_UNEXPECTED_END);

    // Validate payload
    quint32 checksum = Qx::Integrity::crc32(dec);
    if(checksum != hPayloadChecksum)
        return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_CHECKSUM_MISMATCH);

    return Qx::GenericError();
}

}
