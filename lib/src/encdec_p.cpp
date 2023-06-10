// Unit Includes
#include "encdec_p.h"

// Qt Includes
#include <QSize>

// Qx Includes
#include <qx/core/qx-algorithm.h>

namespace PxCrypt
{
/*! @cond */

//-Namespace Functions-------------------------------------------------------------------------------------------------
quint64 calcMaxPayloadBits(const QSize& dim, quint16 tagSize, quint8 bpc)
{
    // Returns 0 if no data can fit
    quint64 pixles = dim.width() * dim.height() - META_PIXELS;
    quint64 headerBits = HEADER_BYTES * 8;
    quint64 tagBits = tagSize * 8;
    quint64 payloadBits = Qx::constrainedSub(pixles * bpc * 3, headerBits + tagBits);

    return payloadBits;
}

quint64 calcMaxPayloadBytes(const QSize& dim, quint16 tagSize, quint8 bpc)
{
    return calcMaxPayloadBits(dim, tagSize, bpc) / 8; // Truncate down
}

/*! @endcond */
}
