#ifndef DECODE_H
#define DECODE_H

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
struct DecodeSettings
{
    QByteArray psk;
    EncType type = Absolute;
};

//-Namespace Functions-------------------------------------------------------------------------------------------------
PXCRYPT_CODEC_EXPORT Qx::GenericError decode(QByteArray& dec, QString& tag, const QImage& enc, DecodeSettings set, const QImage& medium = QImage());

}

#endif // DECODE_H
