#ifndef PX_SEQUENCE_GENERATOR
#define PX_SEQUENCE_GENERATOR

// Qt Includes
#include <QRandomGenerator>
#include <QSize>
#include <QPoint>

// Qx Includes
#include <qx/core/qx-freeindextracker.h>
#include <qx/utility/qx-macros.h>

namespace PxCrypt
{
/*! @cond */

class PxSequenceGenerator
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QSize mDimensions;
    QRandomGenerator mGenerator;
    Qx::FreeIndexTracker mPixelTracker;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    PxSequenceGenerator(const QSize& dim, QByteArrayView seed);

//-Instance Functions----------------------------------------------------------------------------------------------
public:
    int next();
    bool hasNext() const;
};

/*! @endcond */
}

#endif // PX_SEQUENCE_GENERATOR
