// Unit Includes
#include "encdec_p.h"

// Qt Includes
#include <QSize>

// Qx Includes
#include <qx/core/qx-algorithm.h>

namespace PxCrypt
{

//-Namespace Functions-------------------------------------------------------------------------------------------------
quint64 calcMaxPayloadSize(const QSize& dim, quint8 bpc)
{
    // Returns 0 if no data can fit
    quint64 pixles = dim.width() * dim.height() - 1; // -1 For bpp indicator
    quint64 headerBits = (HEADER_BYTES * 8);
    quint64 payloadBits = Qx::constrainedSub(pixles * bpc * 3, headerBits);

    return payloadBits / 8; // Truncate down
}

}
