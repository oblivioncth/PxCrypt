#ifndef _INTERNAL_H
#define _INTERNAL_H

// Qt Includes
#include <QSize>

// Shared Library Support
#include "pxcrypt/pxcrypt_codec_export.h"

/*! @cond */
namespace _PxCrypt
{
//-Namespace Functions-------------------------------------------------------------------------------------------------
PXCRYPT_CODEC_EXPORT quint64 calculateMaximumPayloadBits(const QSize& dim, quint16 tagSize, quint8 bpc);
}
/*! @endcond */

#endif // _INTERNAL_H
