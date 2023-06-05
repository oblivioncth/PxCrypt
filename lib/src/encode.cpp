// Unit Includes
#include "pxcrypt/encode.h"

// Project Includes
#include "pxcrypt/encdec.h"
#include "encdec_p.h"
#include "tools/px_weaver.h"
#include "tools/bit_chunker.h"

// Qx Includes
#include <qx/core/qx-bitarray.h>
#include <qx/core/qx-integrity.h>

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

//-Namespace Functions-------------------------------------------------------------------------------------------------
quint8 calculateOptimalDensity(const QSize& dim, quint16 tagSize, quint32 payloadSize)
{
    // Returns the minimum BPC required to store `payload`, or 0 if the
    // payload will not fit within `dim`
    if(dim.width() == 0 || dim.height() == 0)
        return 0;

    double bits = (payloadSize + tagSize + HEADER_BYTES) * 8.0;
    double chunks = dim.width() * dim.height() * 3.0;
    double bpc = std::ceil(bits/chunks);

    return bpc < 8 ? bpc : 0;
}

quint64 calculateMaximumStorage(const QSize& dim, quint16 tagSize, quint8 bpc)
{
    /* NOTE: Both decode.h and encode.h need the internal version of this function
     * so using a wrapper for the public version prevents including decode.h from
     * automatically including encode.h
     */
    return calcMaxPayloadSize(dim, tagSize, bpc);
}

Qx::GenericError encode(QImage& enc, const QImage& medium, QStringView tag, QByteArrayView payload, EncodeSettings& set)
{
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
            quint64 max = calcMaxPayloadSize(medium.size(), tagData.size(), 7);
            return Qx::GenericError(Qx::GenericError::Critical, ERR_ENCODING_FAILED, ERR_WONT_FIT.arg((payload.size() - max)/1024.0, 0, 'f', 2));
        }
    }

    // Ensure data will fit
    quint64 maxStorage = calcMaxPayloadSize(medium.size(), tagData.size(), set.bpc);
    if(static_cast<quint64>(payload.size()) > maxStorage)
        return Qx::GenericError(Qx::GenericError::Critical, ERR_ENCODING_FAILED, ERR_WONT_FIT.arg((payload.size() - maxStorage)/1024.0, 0, 'f', 2));

    // Create header array
    QByteArray header;
    header.reserve(HEADER_BYTES);

    QDataStream hs(&header, QIODeviceBase::WriteOnly);
    hs.writeRawData(MAGIC_NUM, MAGIC_SIZE);
    hs << EncType::Relative,
    hs << Qx::Integrity::crc32(payload);
    hs << static_cast<quint16>(tagData.size());
    hs << static_cast<quint32>(payload.size());

    // Copy base image
    enc = medium;
    if(enc.colorCount() > 0) // Ensure quadruplet format
        enc.convertTo(QImage::Format_ARGB32);

    // Mark BPC
    Qx::BitArray bpcBits = Qx::BitArray::fromInteger(set.bpc);

    QRgb first = enc.pixel(0, 0);
    quint32 r = (qRed(first) & 0xFFFE) | static_cast<quint32>(bpcBits.testBit(0));
    quint32 g = (qGreen(first) & 0xFFFE) | static_cast<quint32>(bpcBits.testBit(1));
    quint32 b = (qBlue(first) & 0xFFFE) | static_cast<quint32>(bpcBits.testBit(2));
    quint32 a = qAlpha(first);
    enc.setPixel(0, 0, qRgba(r, g, b, a));

    // Prepare pixel weaver
    PxWeaver pWeaver(&enc, set.psk, EncType::Relative);

    // Encode header
    BitChunker bChunker(header, set.bpc);
    while(bChunker.hasNext())
        pWeaver.weave(bChunker.next());

    // Encode Tag
    if(!tagData.isEmpty())
    {
        bChunker = BitChunker(tagData, set.bpc);
        while(bChunker.hasNext())
            pWeaver.weave(bChunker.next());
    }

    // Encode payload
    bChunker = BitChunker(payload, set.bpc);
    while(bChunker.hasNext())
        pWeaver.weave(bChunker.next());

    // Flusher weaver
    pWeaver.flush();

    return Qx::GenericError();
}

}
