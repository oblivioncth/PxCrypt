#ifndef ENCDEC_P_H
#define ENCDEC_P_H

// Qt Includes
#include <QString>
#include <QCryptographicHash>
#include <QSize>
#include <QImage>

using namespace Qt::Literals::StringLiterals;

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

// These are essentially the same as Encoder::Encoding, but exist to keep the usage of that enum strictly within
// the public API so that encoder.h is not a required include in private parts of the code base.
enum EncStrat
{
    Displaced = 0,
    Direct = 4
};

//-Namespace Variables-------------------------------------------------------------------------------------------------
const QByteArray MAGIC_NUM = "PXC"_ba;

const int META_PIXELS = 2; // BPC + EncType

const int MAGIC_SIZE = MAGIC_NUM.size();
const int CHECKSUM_SIZE = sizeof(quint32);
const int TAG_LENGTH_SIZE = sizeof(quint16);
const int PAYLOAD_LENGTH_SIZE = sizeof(quint32);
const int HEADER_BYTES = MAGIC_SIZE +
                         CHECKSUM_SIZE +
                         TAG_LENGTH_SIZE +
                         PAYLOAD_LENGTH_SIZE;

static inline const QByteArray DEFAULT_SEED = "The best and most secure seed that is possible to exist!"_ba;

//-Namespace Functions-------------------------------------------------------------------------------------------------
quint64 calcMaxPayloadBits(const QSize& dim, quint16 tagSize, quint8 bpc);
quint64 calcMaxPayloadBytes(const QSize& dim, quint16 tagSize, quint8 bpc);
QImage standardizeImage(const QImage& img);

/*! @endcond */
}


#endif // ENCDEC_P_H
