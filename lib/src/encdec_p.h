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

namespace pxcrypt
{

//-Namespace Variables-------------------------------------------------------------------------------------------------
const QByteArray MAGIC_NUM = QBAL("PXC");
const QCryptographicHash::Algorithm CHECKSUM_METHOD = QCryptographicHash::Sha256;

const int CHECKSUM_SIZE = QCryptographicHash::hashLength(CHECKSUM_METHOD);
const int HEADER_BYTES = MAGIC_NUM.size() + sizeof(EncType) + CHECKSUM_SIZE + sizeof(quint32);

//-Namespace Functions-------------------------------------------------------------------------------------------------
quint64 calcMaxPayloadSize(const QSize& dim, quint8 bpc);

}


#endif // ENCDEC_P_H
