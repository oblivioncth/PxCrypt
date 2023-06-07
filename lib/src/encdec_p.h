#ifndef ENCDEC_P_H
#define ENCDEC_P_H

// Qt Includes
#include <QString>
#include <QCryptographicHash>
#include <QSize>

// Qx Includes
#include <qx/utility/qx-macros.h>

// Project Includes
#include "pxcrypt/encdec.h"

namespace PxCrypt
{
/*! @cond */

//-Namespace Variables-------------------------------------------------------------------------------------------------
const QByteArray MAGIC_NUM = QBAL("PXC");

const int MAGIC_SIZE = MAGIC_NUM.size();
const int TYPE_SIZE = sizeof(EncType);
const int CHECKSUM_SIZE = sizeof(quint32);
const int TAG_LENGTH_SIZE = sizeof(quint16);
const int PAYLOAD_LENGTH_SIZE = sizeof(quint32);
const int HEADER_BYTES = MAGIC_SIZE +
                         TYPE_SIZE +
                         CHECKSUM_SIZE +
                         TAG_LENGTH_SIZE +
                         PAYLOAD_LENGTH_SIZE;

//-Namespace Functions-------------------------------------------------------------------------------------------------
quint64 calcMaxPayloadSize(const QSize& dim, quint16 tagSize, quint8 bpc);

/*! @endcond */
}


#endif // ENCDEC_P_H
