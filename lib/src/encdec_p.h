#ifndef ENCDEC_P_H
#define ENCDEC_P_H

// Qt Includes
#include <QString>
#include <QCryptographicHash>
#include <QSize>

// Qx Includes
#include <qx/utility/qx-macros.h>

namespace PxCrypt
{
/*! @cond */

//-Namespace Enums-------------------------------------------------------------------------------------------------
enum Channel : quint8{
    Red = 0,
    Green = 1,
    Blue = 2,
    Alpha = 3
};

//-Namespace Variables-------------------------------------------------------------------------------------------------
const QByteArray MAGIC_NUM = QBAL("PXC");

const int META_PIXELS = 2; // BPC + EncType

const int MAGIC_SIZE = MAGIC_NUM.size();
const int CHECKSUM_SIZE = sizeof(quint32);
const int TAG_LENGTH_SIZE = sizeof(quint16);
const int PAYLOAD_LENGTH_SIZE = sizeof(quint32);
const int HEADER_BYTES = MAGIC_SIZE +
                         CHECKSUM_SIZE +
                         TAG_LENGTH_SIZE +
                         PAYLOAD_LENGTH_SIZE;

static inline const QByteArray DEFAULT_SEED = QBAL("The best and most secure seed that is possible to exist!");

//-Namespace Functions-------------------------------------------------------------------------------------------------
quint64 calcMaxPayloadBits(const QSize& dim, quint16 tagSize, quint8 bpc);
quint64 calcMaxPayloadBytes(const QSize& dim, quint16 tagSize, quint8 bpc);

/*! @endcond */
}


#endif // ENCDEC_P_H
