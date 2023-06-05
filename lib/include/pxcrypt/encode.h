#ifndef ENCODE_H
#define ENCODE_H

// Shared Library Support
#include "pxcrypt/pxcrypt_codec_export.h"

// Qt Includes
#include <QImage>

// Qx Includes
#include <qx/core/qx-genericerror.h>

// Project Includes
#include "pxcrypt/encdec.h"

namespace PxCrypt
{
//-Namespace Types ---------------------------------------------------------------------------------------------
struct EncodeSettings
{
    quint8 bpc = 1;
    QStringView psk;
    EncType type = Relative;
};

//-Namespace Functions-------------------------------------------------------------------------------------------------
PXCRYPT_CODEC_EXPORT quint64 calculateMaximumStorage(const QSize& dim, quint16 tagSize, quint8 bpc);
PXCRYPT_CODEC_EXPORT quint8 calculateOptimalDensity(const QSize& dim, quint16 tagSize, quint32 payloadSize);
PXCRYPT_CODEC_EXPORT Qx::GenericError encode(QImage& enc, const QImage& medium, QStringView tag, QByteArrayView payload, EncodeSettings& set);

}

#endif // ENCODE_H
