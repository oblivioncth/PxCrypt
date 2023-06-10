// Unit Includes
#include "pxcrypt/decode.h"

// Project Includes
#include "pxcrypt/encdec.h"
#include "encdec_p.h"
#include "tools/px_access_read.h"
#include "tools/px_skimmer.h"
#include "tools/byte_compositer.h"

// Qx Includes
#include <qx/core/qx-bitarray.h>
#include <qx/core/qx-integrity.h>

// Magic Enum Include
#include <magic_enum.hpp>

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
    const QString ERR_MISSING_MEDIUM = QSL("The encoded image requires a medium to be decoded but none was provided.");
    const QString ERR_DIMMENSION_MISTMATCH = QSL("The required medium image has different dimmensions than the encoded image.");
    const QString ERR_NOT_LARGE_ENOUGH = QSL("The provided image is not large enough to be an encoded image.");
    const QString ERR_NOT_MAGIC = QSL("The provided image is not encoded or the password/medium are incorrect.");
    const QString ERR_LENGTH_MISMATCH = QSL("The encoded image's header indicates it contains more data than possible.");
    const QString ERR_UNEXPECTED_END = QSL("All pixels were skimmed before the expected payload size was reached.");
    const QString ERR_CHECKSUM_MISMATCH = QSL("The payload's checksum did not match the expected value.");

    //-Unit Private Functions ---------------------------------------------------------------------------------------------
    bool canFitHeader(const QSize& dim, quint8 bpc)
    {
        int bits = (dim.width() * dim.height() - META_PIXELS) * 3 * bpc;
        return bits >= HEADER_BYTES * 8;
    }
}

//-Namespace Types-------------------------------------------------------------------------------------------------

//===============================================================================================================
// DecodeSettings
//===============================================================================================================

//-Namespace Functions-------------------------------------------------------------------------------------------------

/*!
 *  Retrieves data from a PxCrypt image.
 *
 *  @param[out] dec The original payload.
 *  @param[out] tag The image's/payload's tag
 *  @param[in] enc The image with the encoded data
 *  @param[in] psk The key that was used when encoding the original data, if any.
 *  @param[in] medium The original medium used when encoding the payload. Ignored for encoding types other than EncType::Relative
 *  @return An error object that describes cause of failure if encoding fails.
 *
 *  @sa encode().
 */
Qx::GenericError decode(QByteArray& dec, QString& tag, const QImage& enc, QByteArrayView psk, const QImage& medium)
{
    // Clear buffer
    dec.clear();

    // Ensure encoded image is valid
    if(enc.isNull())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_INVALID_SOURCE);

    // Ensure image meets bare minimum space for meta pixels
    if(enc.size().width() * enc.size().height() < META_PIXELS)
        return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_NOT_LARGE_ENOUGH);

    // Ensure standard pixel format
    QImage encStd = standardizeImage(enc);

    // Prepare pixel access, automatically reads meta pixels
    PxAccessRead pAccess(&encStd, !psk.isEmpty() ? psk : DEFAULT_SEED);

    // Ensure BPC is valid
    if(pAccess.bpc() < 1 || pAccess.bpc() > 7)
        return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_NOT_MAGIC);

    // Ensure type is valid
    if(!magic_enum::enum_contains(pAccess.type()))
        return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_NOT_MAGIC);

    // Ensure at least header can fit
    if(!canFitHeader(encStd.size(), pAccess.bpc()))
        return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_NOT_LARGE_ENOUGH);

    // Reserve space for header
    dec.reserve(HEADER_BYTES);

    // Ensure medium image is valid if applicable
    QImage mediumStd;
    if(pAccess.type() == EncType::Relative)
    {
        if(medium.isNull())
            return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_MISSING_MEDIUM);

        if(medium.size() != encStd.size())
            return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_DIMMENSION_MISTMATCH);

        mediumStd = standardizeImage(medium);
    }

    // Prepare pixel skimmer and byte compositer
    PxSkimmer pSkimmer(&pAccess, &mediumStd);
    ByteCompositer bCompositer(&dec, pAccess.bpc());

    // Read header
    while(dec.size() != HEADER_BYTES)
        bCompositer.composite(pSkimmer.next());

    // Extract header components
    QDataStream hs(dec);
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
        return Qx::GenericError(Qx::GenericError::Critical, ERR_DECODING_FAILED, ERR_NOT_MAGIC);

    quint64 maxStorage = calcMaxPayloadBytes(encStd.size(), hTagSize, pAccess.bpc());
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
